use gtk::prelude::*;
use std::path::Path;


fn create_window(application: &gtk::Application) {
    let window = gtk::ApplicationWindow::new(application);
    let drawing_area = gtk::DrawingArea::new();
    window.set_child(Some(&drawing_area));

    // Load and scale the image in one line
    let pixbuf = gdk_pixbuf::Pixbuf::from_file_at_scale(&Path::new("/locker/images/svg/maven_Japanese_woman_in_dress.svg"), 100, 100, true).unwrap();

    drawing_area.set_draw_func(move |_, cr,_w, _h| {
        cr.set_source_pixbuf(&pixbuf, 0.0, 0.0);
        let _ = cr.paint();
    });

    window.show();
}

fn main() {
    // Create the application
    let app = gtk::Application::builder()
        .application_id("scale.image.demo")
        .build();
    app.connect_activate(create_window);
    app.run();
}
