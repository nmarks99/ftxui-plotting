#include "ftxui/component/component_base.hpp"
#include "ftxui/screen/color.hpp"
#include <limits>
#include <ftxui/dom/elements.hpp>
#include <ftxui/util/ref.hpp>

std::vector<double> arange(double start, double stop, double step);
constexpr double scale_default_min = -std::numeric_limits<double>::infinity();
constexpr double scale_default_max = std::numeric_limits<double>::infinity();

namespace ftxui {


enum class SeriesStyle {Point, Block};

struct PlotSeries {
    Ref<std::vector<double>> x;
    Ref<std::vector<double>> y;
    Color color;
    SeriesStyle style = SeriesStyle::Point;
};

struct PlotOption {
    Ref<std::vector<PlotSeries>> data;
    Ref<double> xmin = scale_default_min;
    Ref<double> xmax = scale_default_max;
    Ref<double> ymin = scale_default_min;
    Ref<double> ymax = scale_default_max;
};

Component Plot(PlotOption options = {});

} // namespace ftxui
