#include "ftxui/component/component_base.hpp"
#include "ftxui/screen/color.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/util/ref.hpp>

std::vector<double> arange(double start, double stop, double step);

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
    Ref<double> xmin;
    Ref<double> xmax;
    Ref<double> ymin;
    Ref<double> ymax;
};

Component Plot(PlotOption options = {});

} // namespace ftxui
