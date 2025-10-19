#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>

#include "ftxui/component/component_base.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

#include <cassert>
#include <string>
#include <iomanip>

#include "plot.hpp"

std::vector<double> arange(double start, double stop, double step) {
    std::vector<double> out;
    double vi = start;
    while (vi <= stop) {
	out.push_back(vi);
	vi += step;
    }
    return out;
}

std::vector<double> linspace(double start, double stop, size_t num_points) {
    std::vector<double> out(num_points);
    out.at(0) = start;
    const double step = (stop-start) / num_points;
    for (size_t i = 1; i < num_points; i++) {
	out.at(i) = out.at(i-1) + step;
    }
    return out;
}

double linear_map(double value, double a1, double a2, double b1, double b2) {
    if (a2 == a1) {
	return (b1 + b2) / 2;
    }
    const double m = (b2 - b1) / (a2 - a1);
    const double b = b1 - m * a1;
    return m * value + b;
}

namespace ftxui {

class PlotBase : public ComponentBase, public PlotOption {
  public:
    PlotBase(PlotOption option) : PlotOption(std::move(option)) {
        xout_.resize(x().size());
        yout_.resize(y().size());

	// auto-scale if limits are 0.0
	if (xmin() == scale_default_min || xmax() == scale_default_max) {
	    const auto xminmax = std::minmax_element(x().begin(), x().end());
	    xmin = *xminmax.first;
	    xmax = *xminmax.second;
	}
	if (ymin() == scale_default_min || ymax() == scale_default_max) {
	    const auto yminmax = std::minmax_element(y().begin(), y().end());
	    ymin = *yminmax.first;
	    ymax = *yminmax.second;
	}
    }

  private:
    Element OnRender() override {
        auto can = canvas([&](Canvas &c) {

	    constexpr int YTICKS_SPACING = 8;
	    constexpr int XTICKS_SPACING = 12;

	    // draw y ticks
	    auto yticks = arange(ymin(), ymax(), (ymax()-ymin())/c.height());
	    std::reverse(yticks.begin(), yticks.end());
	    for (size_t i = 0; i < c.height()-4; i++) {
		if (i % YTICKS_SPACING == 0) {
		    std::stringstream ss;
		    ss << std::fixed << std::showpos << std::setprecision(2) << yticks.at(i);
		    auto ytick = ss.str();
		    std::replace(ytick.begin(), ytick.end(), '+', ' ');
		    c.DrawText(0, i, ytick +" -");
		}
	    }

	    constexpr int Y_AXIS_OFFSET = 14;

	    // draw x ticks
	    auto xticks = arange(xmin(), xmax(), (xmax()-xmin())/c.width());
	    // std::reverse(xticks.begin(), xticks.end());
	    for (size_t i = Y_AXIS_OFFSET-2; i < c.width(); i++) {
		if (i % XTICKS_SPACING == 0) {
		    std::stringstream ss;
		    ss << std::fixed << std::showpos << std::setprecision(2) << xticks.at(i);
		    auto xtick = ss.str();
		    std::replace(xtick.begin(), xtick.end(), '+', ' ');
		    c.DrawText(i, c.height()-6, "   |");
		    c.DrawText(i, c.height()-4, xtick);
		}
	    }

	    // TODO: this only needs to happen when something changes like
	    // data, canvas size, or axis limits
	    // remake integer data
	    std::transform(x().begin(), x().end(), xout_.begin(), [&](auto v) {
		return static_cast<int>(linear_map(v, xmin(), xmax(), 0+Y_AXIS_OFFSET, c.width()+0));
	    });
	    std::transform(y().begin(), y().end(), yout_.begin(), [&](auto v) {
		return -static_cast<int>(linear_map(v, ymin(), ymax(), 0, c.height() - 5)) + c.height() - 5;
	    });

            // draw line plot
            for (size_t i = 0; i < x().size(); i++) {
                c.DrawPoint(xout_.at(i), yout_.at(i), true, Color::Green);
            }

            canvas_width_last_ = c.width();
            canvas_height_last_ = c.height();
        });
        return can | flex;
    }
    std::vector<int> xout_, yout_;
    double canvas_width_last_ = 0;
    double canvas_height_last_ = 0;
};

Component Plot(PlotOption option) { return Make<PlotBase>(std::move(option)); }
} // namespace ftxui
