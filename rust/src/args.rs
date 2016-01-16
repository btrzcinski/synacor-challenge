use std::env;

pub enum ArgSpec {
    File { filename: String },
    Code { code: String },
    None,
}

pub fn parse_args() -> ArgSpec {
    let mut args = env::args();

    let mode_switch = args.nth(1);
    if mode_switch.is_none() {
        return ArgSpec::None;
    }
    let mode_switch = mode_switch.unwrap();

    let arg = args.next();
    if arg.is_none() {
        return ArgSpec::None;
    }
    let arg = arg.unwrap();
    
    match &mode_switch[..] {
        "-c" => ArgSpec::Code { code: arg },
        "-f" => ArgSpec::File { filename: arg },
        _ => ArgSpec::None
    }
}

pub fn prog_name() -> String {
    env::args().next().unwrap()
}

