#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

#include <algorithm>
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
    const double step = (stop-start) / (num_points-1);
    double val = start;
    for (size_t i = 0; i < num_points; i++) {
	out.at(i) = val;
	val += step;
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

constexpr int YTICKS_SPACING = 8;
constexpr int XTICKS_SPACING = 12;

namespace ftxui {

class PlotBase : public ComponentBase, public PlotOption {
  public:
    PlotBase(PlotOption option) : PlotOption(std::move(option)) {
	// TODO: This just autoscales to first series... okay?
	auto [xtmp, ytmp, color, style] = data().at(0);

	// auto-scale if limits are 0.0
	if (xmin() == scale_default_min || xmax() == scale_default_max) {
	    const auto xminmax = std::minmax_element(xtmp().begin(), xtmp().end());
	    xmin = *xminmax.first;
	    xmax = *xminmax.second;
	}
	if (ymin() == scale_default_min || ymax() == scale_default_max) {
	    const auto yminmax = std::minmax_element(ytmp().begin(), ytmp().end());
	    ymin = *yminmax.first;
	    ymax = *yminmax.second;
	}
    }

  private:

    bool Focusable() const override { return true; }

    // autosales to the first data series
    void auto_scale() {
	auto [xtmp, ytmp, color, style] = data().at(0);
	const auto xminmax = std::minmax_element(xtmp().begin(), xtmp().end());
	xmin() = *xminmax.first;
	xmax() = *xminmax.second;
	const auto yminmax = std::minmax_element(ytmp().begin(), ytmp().end());
	ymin() = *yminmax.first;
	ymax() = *yminmax.second;
    }

    bool OnEvent(Event event) override {
	if (event.is_mouse()
	    && event.mouse().button == Mouse::Left
	    && event.mouse().motion == Mouse::Pressed
	    && box_.Contain(event.mouse().x, event.mouse().y)) {
	    TakeFocus();
	    return true;
	}

	if (!Focused()) {
	    return false;
	}

	if (event == Event::Character('r')) {
	    auto_scale();
	    return true;
	}

	return false;
    }

    Element OnRender() override {
        auto can = canvas([&](Canvas &c) {

	    // TODO: abtract tick drawing/make separate function
	    // draw y ticks
	    auto num_yticks = (c.height()) / YTICKS_SPACING;
	    auto yticks = linspace(ymin(), ymax(), num_yticks);
	    std::reverse(yticks.begin(), yticks.end());
	    for (int i = 0; i < num_yticks; i++) {
		std::stringstream ss;
		ss << std::fixed << std::showpos << std::setprecision(2) << yticks.at(i);
		auto tick = ss.str();
		std::replace(tick.begin(), tick.end(), '+', ' ');
		c.DrawText(0, i*YTICKS_SPACING, tick + "-");
	    }

	    constexpr int Y_AXIS_OFFSET = 14;

	    // draw x ticks
	    auto num_xticks = (c.width()) / XTICKS_SPACING;
	    auto xticks = linspace(xmin(), xmax(), num_xticks);
	    for (int i = 0; i < num_xticks; i++) {
		std::stringstream ss;
		ss << std::fixed << std::showpos << std::setprecision(2) << xticks.at(i);
		auto tick = ss.str();
		std::replace(tick.begin(), tick.end(), '+', ' ');
		c.DrawText(i*XTICKS_SPACING+Y_AXIS_OFFSET-4, c.height()-4, tick);
		c.DrawText(i*XTICKS_SPACING+Y_AXIS_OFFSET-4, c.height()-6, "  |");
	    }

	    // TODO: this only needs to happen when something changes like
	    // data, canvas size, or axis limits
	    for (auto &[x, y, color, style] : *data) {
		std::vector<int> xout(x().size());
		std::vector<int> yout(y().size());
		std::transform(x().begin(), x().end(), xout.begin(), [&](auto v) {
		    return static_cast<int>(linear_map(v, xmin(), xmax(), 0+Y_AXIS_OFFSET, c.width()+0));
		});
		std::transform(y().begin(), y().end(), yout.begin(), [&](auto v) {
		    return -static_cast<int>(linear_map(v, ymin(), ymax(), 0, c.height() - 10)) + c.height() - 10;
		});

		// draw line plot
		if (style == SeriesStyle::Point) {
		    for (size_t i = 1; i < x().size()-1; i++) {
			c.DrawPointLine(xout.at(i-1), yout.at(i-1), xout.at(i), yout.at(i), color);
		    }
		} else if (style == SeriesStyle::Block){
		    for (size_t i = 0; i < x().size(); i++) {
			c.DrawBlock(xout.at(i), yout.at(i), true, color);
		    }
		} else {
		    throw std::runtime_error("Unsupported style");
		}
	    }
            // canvas_width_last_ = c.width();
            // canvas_height_last_ = c.height();
        });
        return can | flex | reflect(box_);
    }
    Box box_;
    // double canvas_width_last_ = 0;
    // double canvas_height_last_ = 0;
};

Component Plot(PlotOption option) { return Make<PlotBase>(std::move(option)); }
} // namespace ftxui
