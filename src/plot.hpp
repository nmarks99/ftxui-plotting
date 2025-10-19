#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/screen/color.hpp"
#include <algorithm>
#include <ftxui/dom/elements.hpp>
#include <ftxui/util/ref.hpp>

std::vector<double> arange(double start, double stop, double step);

namespace ftxui {

constexpr double scale_default_min = -std::numeric_limits<double>::infinity();
constexpr double scale_default_max = -std::numeric_limits<double>::infinity();

struct PlotOption {
    Ref<std::vector<double>> x;
    Ref<std::vector<double>> y;
    Ref<double> xmin = scale_default_min;
    Ref<double> xmax = scale_default_max;
    Ref<double> ymin = scale_default_min;
    Ref<double> ymax = scale_default_max;
};

Component Plot(PlotOption options = {});


} // namespace ftxui
