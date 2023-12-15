#!/bin/sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
rustup target add i686-unknown-linux-gnu
cargo install bindgen-cli
