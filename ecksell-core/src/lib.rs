#[cfg(test)]
mod tests;
use chrono::NaiveDateTime;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ParseContentsError<'a> {
    Todo(&'a str),
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum CellContents<'a> {
    Empty,
    String(&'a str),
    Numeric(isize),
    Monetary(f64),
    Logical(bool),
    Error(ParseContentsError<'a>),
}

pub struct RawCellContents<'a> {
    pub raw: &'a str,
}

pub struct Cell<'a> {
    pub contents: CellContents<'a>,
    pub format: CellFormat,
}

pub enum CellFormat {
    Todo,
}

// TODO: exponential support e.g. 1.5432e10
// TODO: support for non-decimal numbers -> 0xae10f2
// TODO: is there a library that already does this? Can serde be used or something similar?
impl<'a> Into<CellContents<'a>> for RawCellContents<'a> {
    fn into(self) -> CellContents<'a> {
        match self.raw {
            raw_str if raw_str.starts_with('=') => {
                CellContents::Error(ParseContentsError::Todo("formula not yet implemented"))
            }
            raw_str if raw_str.starts_with('\'') => {
                CellContents::Error(ParseContentsError::Todo("formula not yet implemented"))
            }
            raw_str if !raw_str.is_empty() => {
                // @Incomplete -> this will bug out on strings like 0-1 or -.-.
                if raw_str
                    .chars()
                    .all(|c| c.is_digit(10) || c == '.' || c == '-')
                {
                    if raw_str.chars().all(|c| c.is_digit(10) || c == '-') {
                        return CellContents::Numeric(raw_str.parse().unwrap());
                    } else {
                        return CellContents::Monetary(raw_str.parse().unwrap());
                    }
                }
                match raw_str {
                    "true" => CellContents::Logical(true),
                    "false" => CellContents::Logical(false),
                    _ => CellContents::String(raw_str),
                }
            }
            _ => CellContents::Empty,
        }
    }
}

// This owns the underlying data, all Cell type data structures borrow from this.
pub struct RawObject {
    pub raw: String,
    pub object_type: ObjectType,
}

pub enum ObjectType {
    Csv {
        field_delim: char,
        row_delim: char,
        enclosed_by: char,
    },
}

impl Default for ObjectType {
    fn default() -> Self {
        Self::Csv {
            field_delim: ',',
            row_delim: '\n',
            enclosed_by: '"',
        }
    }
}

impl RawObject {
    pub fn new(raw: String) -> Self {
        Self {
            raw,
            object_type: ObjectType::default(),
        }
    }
}
