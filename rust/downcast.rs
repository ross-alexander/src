use std::any::Any;

trait Speaker {
    fn speak(&self);
    fn as_any(&self) -> &dyn Any;
}

struct Dog;

impl Speaker for Dog {
    fn speak(&self) {
        println!("Woof!");
    }
    fn as_any(&self) -> &dyn Any {
        self
    }
}

struct Cat;

impl Speaker for Cat {
    fn speak(&self) {
        println!("Meow!");
    }
    fn as_any(&self) -> &dyn Any {
        self
    }
}

fn downcasting_example() {

    let speakers: [Box<dyn Speaker>; 4] = [
        Box::new(Dog {}),
        Box::new(Cat {}),
        Box::new(Cat {}),
        Box::new(Dog {}),
    ];

    for speaker in speakers {
        speaker.speak();
        if let Some(animal) = speaker.as_any().downcast_ref::<Cat>()
        {
            println!("Catz rulez");
        }
        else
        {
            println!("Dog gone");
        }
    }
    

    
    let dog = Cat;
    dog.speak();

    let speaker: &dyn Speaker = &dog;
    speaker.speak();

    // Using `as_any` to get a `&dyn Any`, then calling `downcast_ref::<Dog>()`
    if let Some(animal) = speaker.as_any().downcast_ref::<Dog>() {
        animal.speak();
    } else {
        println!("This is not a dog!");
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_downcasting() {
        downcasting_example();
    }
}

fn main()
{
    downcasting_example();
}

