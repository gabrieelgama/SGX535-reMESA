// Read-only selected-function decompiler; C output stays in /tmp.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.PrintWriter;

public class DecompileSelected extends GhidraScript {
    @Override
    public void run() throws Exception {
        SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("__driDriverExtensions");
        if (!symbols.hasNext()) throw new IllegalStateException("Missing root symbol");
        long delta = symbols.next().getAddress().getOffset() - 0x2bb1bcL;
        File output = new File("/tmp/sgx535-psb-decompile");
        output.mkdirs();
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        int good = 0, missing = 0, failed = 0;
        try (BufferedReader input = new BufferedReader(new FileReader(
                "/home/gama/sgx535-gfx/tools/psb-dri-re/targets.txt"))) {
            String line;
            while ((line = input.readLine()) != null && !monitor.isCancelled()) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) continue;
                String hex = line.split("\\s+", 2)[0].replace("0x", "");
                long elf = Long.parseUnsignedLong(hex, 16);
                Function f = currentProgram.getFunctionManager().getFunctionAt(toAddr(elf + delta));
                if (f == null || f.isExternal()) {
                    missing++;
                    println("PSB_DECOMP missing function entry 0x" + hex);
                    continue;
                }
                File file = new File(output, String.format("%08x.txt", elf));
                if (file.exists()) { good++; continue; }
                DecompileResults result = dec.decompileFunction(f, 25, monitor);
                if (!result.decompileCompleted() || result.getDecompiledFunction() == null) {
                    failed++; println("PSB_DECOMP failed 0x" + hex); continue;
                }
                try (PrintWriter pw = new PrintWriter(new FileWriter(file))) {
                    pw.println("ELF VA 0x" + hex + " Ghidra " + f.getEntryPoint() + " " + f.getName());
                    pw.println(result.getDecompiledFunction().getC());
                }
                good++;
                println("PSB_DECOMP wrote 0x" + hex);
            }
        }
        dec.dispose();
        println("PSB_DECOMP complete=" + good + " missing=" + missing + " failed=" + failed);
    }
}
