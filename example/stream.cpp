#include <cmath>
#include <string>
#include <deque>
#include <thread>
#include <random>

#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <ftxui-plot/plot.hpp>

using namespace ftxui;

using PlotData = std::vector<PlotSeries<std::deque<double>>>;

// note not thread safe
double get_random(double min_val, double max_val) {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    std::uniform_real_distribution<double> distribution(min_val, max_val);
    return distribution(generator);
}

int main() {

    auto screen = ScreenInteractive::Fullscreen();

    // Create some data
    std::deque<double> x1 = arange<std::deque<double>>(0, 5, 0.05);
    std::deque<double> y1(x1.size(), 100.0); // hack
    Color color1 = Color::Red;

    PlotData data = {
	{&x1, &y1, &color1, SeriesStyle::PointLine},
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

    // Axis limits
    std::string ymin = "-2.0";
    std::string ymax = "2.0";
    std::string xmin = "0.0";
    std::string xmax = "5.0";

    auto make_input = [&](std::string &str){
	auto op = InputOption{};
	op.multiline = false;
	op.content = &str;
	return Input(op);
    };
    auto ymin_inp = make_input(ymin);
    auto ymax_inp = make_input(ymax);
    auto xmin_inp = make_input(xmin);
    auto xmax_inp = make_input(xmax);

    // Create the plot component
    PlotOption<std::deque<double>> op;
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
//
    // // Auto-scale on start
    // plot->OnEvent(PlotEvent::AutoScale);

    // main program loop
    constexpr int POLL_PERIOD_MS = 50;
    Loop loop(&screen, main_renderer);
    while (!loop.HasQuitted()) {

	y1.push_back(get_random(-1.0, 1.0));
	y1.pop_front();

	screen.PostEvent(Event::Custom);

	loop.RunOnce();
	std::this_thread::sleep_for(std::chrono::milliseconds(POLL_PERIOD_MS));
    }

    return 0;
}
