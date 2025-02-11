use relm::{connect, Relm, Update, Widget};
use gtk::prelude::*;
use gtk::{Window, Inhibit, WindowType};
use relm_derive::{Msg, widget};
use thiserror::Error;

pub struct Model {
    counter: isize
}

#[derive(Msg)]
pub enum Msg {
    Increment,
    Decrement,
    Quit,
}


#[widget]
impl Widget for Win {
    fn model() -> Model {
        Model {
            counter: 0,
        }
    }

    fn update(&mut self, event: Msg) {
        match event {
            // A call to self.label1.set_text() is automatically inserted by the
            // attribute every time the model.counter attribute is updated.
            Msg::Decrement => self.model.counter -= 1,
            Msg::Increment => self.model.counter += 1,
            Msg::Quit => gtk::main_quit(),
        }
    }

    view! {
        gtk::Window {
            gtk::Box {
                orientation: gtk::Orientation::Vertical,
                gtk::Button {
                    // By default, an event with one paramater is assumed.
                    clicked => Msg::Increment,
                    // Hence, the previous line is equivalent to:
                    // clicked(_) => Increment,
                    label: "+",
                },
                gtk::Label {
                    // Bind the text property of this Label to the counter attribute
                    // of the model.
                    // Every time the counter attribute is updated, the text property
                    // will be updated too.
                    text: &self.model.counter.to_string(),
                },
                gtk::Button {
                    clicked => Msg::Decrement,
                    label: "-",
                },
            },
            // Use a tuple when you want to both send a message and return a value to
            // the GTK+ callback.
            delete_event(_, _) => (Msg::Quit, Inhibit(false)),
        }
    }
}


fn main() -> Result<(), XlErr> {
    Win::run(())?;

    Ok(())
}

#[derive(Debug, Error)]
pub enum XlErr {
    #[error(transparent)]
    GLibErr(#[from] glib::BoolError)
}

