/* ----------------------------------------------------------------------
--
-- rust program to compress images using cairo
--
-- 2023-05-06: Initial working version
--
---------------------------------------------------------------------- */

use std::env;
use std::process;
use std::fs;
use std::path::PathBuf;

use rand::Rng;

// Requires gtk::cairo rather than just cairo as we are using gdk-pixbuf
// for image loading

use gtk::cairo::{Context,ImageSurface,Format};
use gdk_pixbuf::Pixbuf;
use gdk::prelude::GdkContextExt;
// use gdk::prelude::GdkPixbufExt;
use gdk;


struct RsImage {
    path: String,
    format: String,
    width: i32,
    height: i32,
    buffer: Option<Pixbuf>
}

/* ----------------------------------------------------------------------
--
-- find_images
--
---------------------------------------------------------------------- */

fn find_images(path: &PathBuf, images: &mut Vec<RsImage>)
{
    let files = fs::read_dir(path).unwrap();
    for f in files {
        let file: PathBuf = f.unwrap().path();
//        println!("{}", file.display());
        if file.is_dir()
        {
            find_images(&file, images);
        }
        if file.is_file()
        {
            let info = Pixbuf::file_info(&file);
            match info
            {
                Some((format, width, height)) => {
                    match format.name()
                    {
                        None => println!("Missing format."),
                        Some(name) => {
//                            println!("Found {}, {} x {}", name, width, height);
                            let image :RsImage = RsImage {
                                path: file.display().to_string(),
                                format: name.to_string(),
                                width: width,
                                height: height,
                                buffer: None,
                            };
                            images.push(image);
                        },
                    }
                },
                None => {}, // println!("Not an image file."),
            }
        }
    }
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

fn main()
{
// need to call gdk::init, which required DISPLAY to be set
    
    gdk::init();
    
    // collect iterator into vector
    let mut paths: Vec<PathBuf> = Vec::new();
    let args: Vec<String> = env::args().collect();
    let mut images: Vec<RsImage> = Vec::new();

    // require atleast one argument
    if args.len() < 2
    {
        eprintln!("{}: <path list>", args[0]);
        process::exit(1);
    }

// Loop over arguments
    
    for argument in args
    {
        let path = PathBuf::from(&argument);
        if path.exists() && path.is_dir()
        {
            paths.push(path);
        }
    }

// Find any legitimate images on these paths (recursive)
    
    for p in paths
    {
        find_images(&p, &mut images)
    }
    
    /*
    for i in &images {
        println!("{:5} {:6} × {:6} {}", i.format, i.width, i.height, i.path);
    }
     */

    // Pick a random image
    
    let mut rng = rand::thread_rng();
    let index = rng.gen_range(0 .. images.len());
    let image = &mut images[index];

    println!("{:5} {:6} × {:6} {}", image.format, image.width, image.height, image.path);


    // Load image into GDK Pixbuf

//    let buffer : Pixbuf = Pixbuf::from_file(&image.path).unwrap();
    
    image.buffer = Some(Pixbuf::from_file(&image.path).unwrap());

    // Create Cairo image surface

    let surface = ImageSurface::create(Format::ARgb32, image.width, image.height).expect("Couldn't not create surface");
    let context : gdk::cairo::Context = Context::new(&surface).expect("Could not create context");

    match &image.buffer {
        Some(buffer) => context.set_source_pixbuf(buffer, 0.0, 0.0),
        None => context.set_source_rgb(0.0, 0.0, 0.0),
    }
    context.paint().unwrap();

    let out_pixbuf = gdk::pixbuf_get_from_surface(&surface, 0, 0, image.width, image.height).unwrap();

    out_pixbuf.savev("output.jpeg", "jpeg", &[]).expect("Could not write file");
    
//    let mut stream = File::create("output.jpeg").expect("Could not create output file");
//    surface.write_to_png(&mut stream).expect("Could not write PNG");
}
