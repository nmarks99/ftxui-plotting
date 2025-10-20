#include <cmath>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <string>

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include "plot.hpp"

using namespace ftxui;

int main() {

    auto screen = ScreenInteractive::Fullscreen();

    // Create some data
    auto x = arange(0, 2 * M_PI, 0.01);
    std::vector<double> y(x.size());
    std::transform(x.begin(), x.end(), y.begin(), [](double v) { return std::sin(v); });

    double ymin = -1.0;
    std::string ymin_str = std::to_string(ymin);
    double ymax = 1.0;
    std::string ymax_str = std::to_string(ymax);

    double xmin = 0.0;
    std::string xmin_str = std::to_string(xmin);
    double xmax = 2*M_PI;
    std::string xmax_str = std::to_string(xmax);

    auto ymin_inp = Input(InputOption{
	.content = &ymin_str,
	.multiline = false,
	.on_change = [&]{
	    try {
		ymin = std::stod(ymin_str);
	    } catch (...) {}
	}
    });
    auto ymax_inp = Input(InputOption{
	.content = &ymax_str,
	.multiline = false,
	.on_change = [&]{
	    try {
		ymax = std::stod(ymax_str);
	    } catch (...) {}
	}
    });
    auto xmin_inp = Input(InputOption{
	.content = &xmin_str,
	.multiline = false,
	.on_change = [&]{
	    try {
		xmin = std::stod(xmin_str);
	    } catch (...) {}
	}
    });
    auto xmax_inp = Input(InputOption{
	.content = &xmax_str,
	.multiline = false,
	.on_change = [&]{
	    try {
		xmax = std::stod(xmax_str);
	    } catch (...) {}
	}
    });

    // Create the plot component
    PlotOption op;
    op.x = x;
    op.y = y;
    op.xmin = &xmin;
    op.xmax = &xmax;
    op.ymin = &ymin;
    op.ymax = &ymax;
    auto plot = Plot(op);

    // // Main container to define interactivity of components
    auto main_container = Container::Vertical({
	// plot,
	ymin_inp,
	ymax_inp,
	xmin_inp,
	xmax_inp
    });

    // Main renderer to define visual layout of components and elements
    auto main_renderer = Renderer(main_container, [&] {
	return vbox({
	    plot->Render() | border,
	    vbox({
		hbox({
		    text("X Range: "),
		    xmin_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
		    separatorEmpty(),
		    xmax_inp->Render() | size(WIDTH, EQUAL, 10),
		}),
		separatorEmpty(),
		hbox({
		    text("Y Range: "),
		    ymin_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
		    separatorEmpty(),
		    ymax_inp->Render() | size(WIDTH, EQUAL, 10),
		}),
	    }) | border
	});
    });

    screen.Loop(main_renderer);

    return 0;
}
