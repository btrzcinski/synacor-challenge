mod args;
use args::*;

fn main() {
    match parse_args() {
        ArgSpec::None => println!("Usage: {} [-f|-c] [filename|code]", prog_name()),
        ArgSpec::Code { code } => println!("Interpreting code: {}", code),
        ArgSpec::File { filename } => println!("Interpreting binary file: {}", filename),
    }
}
