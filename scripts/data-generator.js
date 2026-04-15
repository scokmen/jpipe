const fs = require('fs');
const {Buffer} = require('node:buffer');

const RecordSeparator = Buffer.from([0x1E]);
const EscapeCharacters = ['"', '\\', '\n', '\r', '\t', '\b', '\f'];
const SafeCharacters = 'abcdefgh ijklmnop qrstuvwx yzABCDEF GHIJKLMN OPQRSTUV WXYZ0123 456789.-(){}';
const MultiByteCharacters = ['🚀', '🔥', '💎', '🌈', '∑', '∞', '∆', '≈', '©', '®', '±', '§'];

function generateBenchmarkData(fileName, count, recordLength, escapeRatio) {
    const stream = fs.createWriteStream(fileName);

    for (let i = 0; i < count; i++) {
        const currentLogLen = Math.floor(recordLength * (0.8 + Math.random() * 0.4));
        const logBuffer = Buffer.allocUnsafe(currentLogLen * 6);
        let pos = 0;

        for (let j = 0; j < currentLogLen; j++) {
            const roll = Math.random() * 100;
            if (escapeRatio > 0 && roll < escapeRatio) {
                const char = (Math.random() > 0.25)
                    ? EscapeCharacters[Math.floor(Math.random() * EscapeCharacters.length)]
                    : MultiByteCharacters[Math.floor(Math.random() * MultiByteCharacters.length)];
                pos += logBuffer.write(char, pos);
            } else {
                const char = SafeCharacters[Math.floor(Math.random() * SafeCharacters.length)];
                logBuffer[pos++] = char.charCodeAt(0);
            }
        }

        stream.write(logBuffer.subarray(0, pos));
        stream.write(RecordSeparator);
    }

    stream.end();
}

function showHelpAndAbort() {
    console.log(`
Usage: 
  node data-generator.js <file> <count> <recordLength> <escapeRatio>

Arguments:
  file         : Output file path (e.g., data.bin)
  count        : Total number of records (e.g., 100000)
  recordLength : Average length of each record (e.g., 256)
  escapeRatio  : Percentage of escape/multi-byte chars (0-100)

Example:
  node data-generator.js test.bin 100 512 7
`);
    process.exit(1);
}

const args = process.argv.slice(2);

if (args.length !== 4) {
    showHelpAndAbort();
} else {
    const fileName = args[0];
    const count = parseInt(args[1], 10);
    const recordLength = parseInt(args[2], 10);
    const escapeRatio = parseFloat(args[3]);

    if (!fileName || isNaN(count) || isNaN(recordLength) || isNaN(escapeRatio)) {
        console.error(' ❌ Error: Missing or invalid arguments.');
        showHelpAndAbort();
    }

    if (count < 10 || count > 10000) {
        console.error(' ❌ Error: Count must be between 10 and 1000');
        showHelpAndAbort();
    }

    if (recordLength < 16 || recordLength > 1024) {
        console.error(' ❌ Error: Record length must be between 16 and 1024');
        showHelpAndAbort();
    }

    if (escapeRatio < 0 || escapeRatio > 100) {
        console.error(" ❌ Error: escapeRatio must be between 0 and 100");
        showHelpAndAbort();
    }

    generateBenchmarkData(fileName, count, recordLength, escapeRatio);
}
