import sys
import re
import json

output_file = sys.argv[1]

class KConfigReader:
    def __init__(self, file):
        self._config = {}
        with open(file, "r", encoding="utf-8") as f:
            for line in f:
                m = re.match(
                    r"^(?P<key>[^=\n]+)=(?:(?P<v_bool>y)|(?P<v_int>\d+)|(?P<v_hex>0x[0-9A-Fa-f]+)|(?:\"(?P<v_string>[^\"\n]*)\"))$",
                    line,
                )
                if m is None:
                    continue
                val = None
                for g in [
                    ("v_bool", lambda x: x == "y"),
                    ("v_int", int),
                    ("v_hex", lambda x: int(x, 16)),
                    ("v_string", str),
                ]:
                    v = m.group(g[0])
                    if v is None:
                        continue
                    val = g[1](v)
                self._config[m.group("key")] = val

    def __getitem__(self, key):
        return self._config.get(key)

def main():
    config = KConfigReader(".config")
    target = {
        "arch": config["CONFIG_ARCH"],
    }
    match config["CONFIG_ARCH"]:
        case "x86":
            target = target | {
                "rustc-abi": "x86-softfloat",
                "data-layout": "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-i128:128-f64:32:64-f80:32-n8:16:32-S128",
                "features": "-mmx,+soft-float",
                "llvm-target": "i386-unknown-linux-gnu",
                "target-pointer-width": 32,
                "stack-probes": {
                    "kind": "none"
                }
            }
        case _:
            raise Exception(f"unsupported architecture {config['CONFIG_ARCH']}")

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(json.dumps(target, indent=2))

if __name__ == "__main__":
    main()