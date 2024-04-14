mod core;

use super::*;

macro_rules! cell_parser_test {
    ($($result:expr => $expectation:expr,)+) => {
        #[test]
        fn cell_parser_test() {
            $(
                let sut = RawCellContents{ raw: $result };
                let cc: CellContents = sut.into();
                assert_eq!(cc, $expectation);
            )+
        }
    };
}

cell_parser_test!(
    "" => CellContents::Empty,
    "a" => CellContents::String("a"),
    "100" => CellContents::Numeric(100_isize),
    "-100" => CellContents::Numeric(-100_isize),
    "1.5" => CellContents::Monetary(1.5_f64),
    "true" => CellContents::Logical(true),
    "false" => CellContents::Logical(false),
    "=false" => CellContents::Error(ParseContentsError::Todo("formula not yet implemented")),
);
