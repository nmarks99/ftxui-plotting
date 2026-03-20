#pragma once
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <string>

namespace ftxui::detail {
template <typename T>
inline std::optional<T> get_as(const std::string& str) {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, double>, "Invalid type T");
    T value{};
    auto first = str.data();
    auto last = str.data() + str.size();
    auto [ptr, err] = std::from_chars(first, last, value);
    if (err == std::errc() && ptr == last) {
        return value;
    } else {
        return std::nullopt;
    }
}

inline std::vector<double> linspace(double start, double stop, size_t num_points) {
    std::vector<double> out(num_points);
    const double step = (stop - start) / (num_points - 1);
    double val = start;
    for (size_t i = 0; i < num_points; i++) {
        out.at(i) = val;
        val += step;
    }
    return out;
}

inline double linear_map(double value, double a1, double a2, double b1, double b2) {
    if (a2 == a1) {
        return (b1 + b2) / 2;
    }
    const double m = (b2 - b1) / (a2 - a1);
    const double b = b1 - m * a1;
    return m * value + b;
}

constexpr int YTICKS_SPACING = 8;
constexpr int XTICKS_SPACING = 12;
} // namespace ftxui::detail

template <class Container>
inline Container arange(double start, double stop, double step) {
    Container out;
    double vi = start;
    while (vi <= stop) {
        out.push_back(vi);
        vi += step;
    }
    return out;
}

namespace ftxui {

namespace PlotEvent {
inline const ftxui::Event AutoScale = ftxui::Event::Special("PLOT_AUTOSCALE");
}

enum class SeriesStyle { PointLine, BlockLine, PointScatter, BlockScatter };

template <class Container>
struct PlotSeries {
    Ref<Container> x;
    Ref<Container> y;
    Ref<Color> color = Color::RGB(0, 0, 255);
    Ref<SeriesStyle> style = SeriesStyle::PointLine;
};

template <class Container>
struct PlotOption {
    Ref<std::vector<PlotSeries<Container>>> data;
    StringRef xmin;
    StringRef xmax;
    StringRef ymin;
    StringRef ymax;
    Ref<bool> show_x_ticks = true;
    Ref<bool> show_y_ticks = true;
};

template <class Container>
Component Plot(PlotOption<Container> options = {});

template <class Container>
class PlotBase : public ComponentBase, public PlotOption<Container> {
  public:
    PlotBase(PlotOption<Container> option) : PlotOption<Container>(std::move(option)) {}

  private:
    double xmin_ = 0.0;
    double xmax_ = 0.0;
    double ymin_ = 0.0;
    double ymax_ = 0.0;
    Box box_;

    std::vector<int> xout_;
    std::vector<int> yout_;

    bool Focusable() const override { return true; }

    void get_plot_limits_double() {
        if (auto v = detail::get_as<double>(this->xmin()); v.has_value()) {
            xmin_ = v.value();
        }
        if (auto v = detail::get_as<double>(this->xmax()); v.has_value()) {
            xmax_ = v.value();
        }
        if (auto v = detail::get_as<double>(this->ymin()); v.has_value()) {
            ymin_ = v.value();
        }
        if (auto v = detail::get_as<double>(this->ymax()); v.has_value()) {
            ymax_ = v.value();
        }
    }

    void set_plot_limits_string() {
        this->xmin() = std::to_string(xmin_);
        this->xmax() = std::to_string(xmax_);
        this->ymin() = std::to_string(ymin_);
        this->ymax() = std::to_string(ymax_);
    }

    void auto_scale() {
        xmin_ = std::numeric_limits<double>::infinity();
        xmax_ = -std::numeric_limits<double>::infinity();
        ymin_ = std::numeric_limits<double>::infinity();
        ymax_ = -std::numeric_limits<double>::infinity();
        for (const auto& [xtmp, ytmp, color, style] : this->data()) {
            if (!xtmp->empty()) {
                auto [min_it, max_it] = std::minmax_element(xtmp->begin(), xtmp->end());
                xmin_ = std::min(*min_it, xmin_);
                xmax_ = std::max(*max_it, xmax_);
            }
            if (!ytmp->empty()) {
                auto [min_it, max_it] = std::minmax_element(ytmp->begin(), ytmp->end());
                ymin_ = std::min(*min_it, ymin_);
                ymax_ = std::max(*max_it, ymax_);
            }
        }
        set_plot_limits_string();
    }

    bool OnEvent(Event event) override {
        if (event.is_mouse() && event.mouse().button == Mouse::Left &&
            event.mouse().motion == Mouse::Pressed && box_.Contain(event.mouse().x, event.mouse().y)) {
            TakeFocus();
            return true;
        }

        if (event == PlotEvent::AutoScale) {
            auto_scale();
            return true;
        }

        return false;
    }

    int draw_ticks(Canvas& c) {

        int x_start = 0;
        if (this->show_y_ticks()) {
            auto num_yticks = (c.height()) / detail::YTICKS_SPACING;
            auto yticks_d = detail::linspace(ymin_, ymax_, num_yticks);
            std::vector<std::string> yticks(yticks_d.size());
            std::transform(yticks_d.begin(), yticks_d.end(), yticks.begin(), [](auto s) {
                std::stringstream ss;
                ss << std::fixed << std::showpos << std::setprecision(2) << s;
                auto str = ss.str();
                std::replace(str.begin(), str.end(), '+', ' ');
                return str;
            });
            std::reverse(yticks.begin(), yticks.end());

            // draw the Y ticks
            for (int i = 0; i < num_yticks; i++) {
                c.DrawText(0, i * detail::YTICKS_SPACING, yticks.at(i) + "-");
            }

            // Compute the x offset
            auto longest_tick = std::max_element(
                yticks.begin(), yticks.end(),
                [](const std::string& a, const std::string& b) { return a.length() < b.length(); });
            x_start = (longest_tick->length() + 4) * 2;
        }

        if (this->show_x_ticks()) {
            auto num_xticks = (c.width()) / detail::XTICKS_SPACING;
            auto xticks = detail::linspace(xmin_, xmax_, num_xticks);
            for (int i = 0; i < num_xticks; i++) {
                std::stringstream ss;
                ss << std::fixed << std::showpos << std::setprecision(2) << xticks.at(i);
                auto tick = ss.str();
                std::replace(tick.begin(), tick.end(), '+', ' ');
                c.DrawText(i * detail::XTICKS_SPACING + x_start - 4, c.height() - 4, tick);
                c.DrawText(i * detail::XTICKS_SPACING + x_start - 4, c.height() - 6, "  |");
            }
        }

        return x_start;
    }

    Element OnRender() override {
        get_plot_limits_double();

        auto can = canvas([&](Canvas& c) {
            int x_plot_start = draw_ticks(c);
            int y_plot_start = c.height() - 8;

            auto this_data = this->data();
            for (size_t i = 0; i < this_data.size(); i++) {
                auto& [x, y, color, style] = this_data[i];
                xout_.resize(x().size());
                yout_.resize(y().size());
                auto& xout = xout_;
                auto& yout = yout_;
                std::transform(x().begin(), x().end(), xout.begin(), [&](auto v) {
                    return static_cast<int>(
                        detail::linear_map(v, xmin_, xmax_, 0 + x_plot_start, c.width() + 0));
                });
                std::transform(y().begin(), y().end(), yout.begin(), [&](auto v) {
                    if (std::isnan(v)) {
                        return std::numeric_limits<int>::max();
                    }
                    return -static_cast<int>(detail::linear_map(v, ymin_, ymax_, 0, c.height() - 10)) +
                           c.height() - 10;
                });

                auto in_view = [&](int x, int y) -> bool {
                    if (x == std::numeric_limits<int>::max() || y == std::numeric_limits<int>::max()) {
                        return false;
                    }
                    return (x >= x_plot_start) && (y < y_plot_start);
                };

                // plot the data
                switch (style()) {
                case SeriesStyle::PointLine:
                    for (size_t i = 1; i < x().size(); i++) {
                        if (!in_view(xout[i], yout[i]) || !in_view(xout[i - 1], yout[i - 1])
                                || ((xout[i-1] == xout[i]) && (yout[i-1] == yout[i]))) {
                            continue;
                        }
                        c.DrawPointLine(xout[i - 1], yout[i - 1], xout[i], yout[i], color());
                    }
                    break;
                case SeriesStyle::PointScatter:
                    for (size_t i = 0; i < x().size(); i++) {
                        if (!in_view(xout[i], yout[i])) {
                            continue;
                        }
                        c.DrawPoint(xout[i], yout[i], true, color());
                    }
                    break;
                case SeriesStyle::BlockLine:
                    for (size_t i = 1; i < x().size(); i++) {
                        if (!in_view(xout[i], yout[i]) || !in_view(xout[i - 1], yout[i - 1])
                                || ((xout[i-1] == xout[i]) && (yout[i-1] == yout[i]))) {
                            continue;
                        }
                        c.DrawBlockLine(xout[i - 1], yout[i - 1], xout[i], yout[i], color());
                    }
                    break;
                case SeriesStyle::BlockScatter:
                    for (size_t i = 0; i < x().size(); i++) {
                        if (!in_view(xout[i], yout[i])) {
                            continue;
                        }
                        c.DrawBlock(xout[i], yout[i], true, color());
                    }
                    break;
                }
            }
        });
        return can | flex | reflect(box_);
    }
};

template <class Container>
Component Plot(PlotOption<Container> option) {
    return Make<PlotBase<Container>>(std::move(option));
}

} // namespace ftxui
