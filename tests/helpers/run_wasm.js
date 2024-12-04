const fs = require("fs");

const outputPath = "./output.txt";
fs.writeFileSync(outputPath, "");

const moduleOptions = {
    env: {
        print_i32: (value) => {
            console.log(value);
            fs.appendFileSync(outputPath, value.toString() + "\n");
        },
        print_f64: (value) => {
            console.log(value);
            fs.appendFileSync(outputPath, value.toString() + "\n");
        }
    }
};

const result = fs.readFileSync("output.wasm");

WebAssembly.instantiate(result, moduleOptions).then((instance) => {
    instance.instance.exports.main();
});