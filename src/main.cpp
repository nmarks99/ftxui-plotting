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

using PlotData = std::vector<PlotSeries>;

int main() {

    auto screen = ScreenInteractive::Fullscreen();

    // Create some data
    auto x1 = arange(0, 4 * M_PI, 0.1);
    std::vector<double> y1(x1.size());
    std::transform(x1.begin(), x1.end(), y1.begin(), [](double v) { return 2*std::sin(v); });
    Color color1 = Color::Red;

    auto x2 = arange(0, 8 * M_PI, 0.1);
    std::vector<double> y2(x2.size());
    std::transform(x2.begin(), x2.end(), y2.begin(), [](double v) { return 2.0/3.0*std::cos(v); });
    Color color2 = Color::Blue;

    PlotData data = {
	{&x1, &y1, &color1},
	{&x2, &y2}
    };

    // Color selector for series 1
    std::vector<std::string> color1_entries{"Red", "Orange", "Purple", "Green"};
    int color1_choice = 0;
    auto color1_radio_op = RadioboxOption{};
    color1_radio_op.entries = color1_entries;
    color1_radio_op.selected = &color1_choice;
    color1_radio_op.on_change = [&](){
	switch (color1_choice) {
	    case 0:
		color1 = Color::Red;
		break;
	    case 1:
		color1 = Color::Orange1;
		break;
	    case 2:
		color1 = Color::Purple;
		break;
	    case 3:
		color1 = Color::Green;
		break;
	}
    };
    auto color1_menu = Radiobox(color1_radio_op);

    double lastx = x1.back();
    double lasty = y1.back();
    for (size_t i = 0; i < 50; i++) {
	lastx += 0.1;
	x1.push_back(lastx);
	y1.push_back(lasty);
    }

    // axis limits
    // TODO: synchronize doubles and strings here
    // Idea: catch auto-scale event and do to_string then?
    double ymin = 0.0;
    std::string ymin_str = std::to_string(ymin);
    double ymax = 0.0;
    std::string ymax_str = std::to_string(ymax);
    double xmin = 0.0;
    std::string xmin_str = std::to_string(xmin);
    double xmax = 0.0;
    std::string xmax_str = std::to_string(xmax);

    // Input components for axis limits
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
    op.data = &data;
    op.xmin = &xmin;
    op.xmax = &xmax;
    op.ymin = &ymin;
    op.ymax = &ymax;
    auto plot = Plot(op);

    // autoscale button
    auto button_op = ButtonOption::Simple();
    button_op.label = "Auto-Scale";
    button_op.on_click = [&](){
	plot->OnEvent(PlotEvent::AutoScale);
    };
    auto autoscale_button = Button(button_op);

    // Main container to define interactivity of components
    auto main_container = Container::Vertical({
	plot,
	ymin_inp,
	ymax_inp,
	xmin_inp,
	xmax_inp,
	color1_menu,
	autoscale_button
    });

    // Main renderer to define visual layout of components and elements
    auto main_renderer = Renderer(main_container, [&] {
	return vbox({
	    plot->Render() | (border | (plot->Active() ? color(Color::LightSkyBlue1) : color(Color::White))),
	    hbox({
		vbox({
		    text("Axis limits") | underlined,
		    hbox({
			text("X Range: "),
			xmin_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
			separatorEmpty(),
			xmax_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
		    }),
		    hbox({
			text("Y Range: "),
			ymin_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
			separatorEmpty(),
			ymax_inp->Render() | size(WIDTH, EQUAL, 10) | bgcolor(Color::RGB(50,50,50)),
		    }),
		    separatorEmpty(),
		    autoscale_button->Render() | size(WIDTH, EQUAL, 12),
		}) | borderEmpty,
		separator(),
		vbox({
		    text("Series 1") | underlined,
		    color1_menu->Render(),
		}) | borderEmpty,
	    }) | border | size(HEIGHT, EQUAL, 12),
	});
    });

    // Auto-scale on start
    plot->OnEvent(PlotEvent::AutoScale);

    screen.Loop(main_renderer);

    return 0;
}
