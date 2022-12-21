extern crate gdk_pixbuf;

use std::env;
use std::process;
use std::fs;
// use std::path::Path;
use std::path::PathBuf;
use gdk_pixbuf::Pixbuf;

struct RsImage {
    path: String,
    format: String,
    width: i32,
    height: i32
}

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
                                height: height
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

fn main()
{

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
    
    for argument in args
    {
        let path = PathBuf::from(&argument);
        if path.exists() && path.is_dir()
        {
            paths.push(path);
        }
    }

    for p in paths
    {
        show_paths(&p, &mut images)
    }

    for i in images {
        println!("{:5} {:6} × {:6} {}", i.format, i.width, i.height, i.path);
    }
}
