// Read-only Ghidra analysis of the retained Xpsb.so in a disposable project.
// Output stays in /tmp; this script never invokes the ELF or accesses hardware.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.*;

public class TraceStaticBuffer extends GhidraScript {
    public void run() throws Exception {
        SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("XpsbInit");
        if (!symbols.hasNext()) throw new IllegalStateException("XpsbInit missing");
        long delta = symbols.next().getAddress().getOffset() - 0x2690L;
        File output = new File("/tmp/sgx535-xpsb-static-trace.txt");
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        try (PrintWriter out = new PrintWriter(new FileWriter(output))) {
            out.println("ELF source range: 0xd0e0..0xd297; Ghidra delta: 0x" + Long.toHexString(delta));
            int totalRefs = 0;
            for (long va = 0xd0e0; va < 0xd0e0 + 0x1b8; va++) {
                Address address = toAddr(va + delta);
                ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(address);
                while (refs.hasNext()) {
                    Reference ref = refs.next();
                    out.printf("DATA_REF target=0x%x source=0x%x type=%s%n", va,
                        ref.getFromAddress().getOffset() - delta, ref.getReferenceType());
                    totalRefs++;
                }
            }
            out.println("DATA_REF_COUNT=" + totalRefs);
            Address shaderAccessor = toAddr(0x2670 + delta);
            ReferenceIterator shaderRefs = currentProgram.getReferenceManager().getReferencesTo(shaderAccessor);
            while (shaderRefs.hasNext()) {
                Reference ref = shaderRefs.next();
                out.printf("SHADER_ACCESSOR_REF source=0x%x type=%s%n",
                    ref.getFromAddress().getOffset() - delta, ref.getReferenceType());
            }
            int count = 0;
            FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
            while (functions.hasNext() && !monitor.isCancelled()) {
                Function f = functions.next();
                if (f.isExternal()) continue;
                count++;
                DecompileResults result = dec.decompileFunction(f, 12, monitor);
                if (!result.decompileCompleted() || result.getDecompiledFunction() == null) continue;
                String body = result.getDecompiledFunction().getC();
                for (String line : body.split("\\R")) {
                    if (line.contains("[0x1f]") || line.contains("+ 0x7c") ||
                        line.contains("USSE static buffer") || line.contains("DAT_0001d0e0") ||
                        line.contains("XpsbShaderCode") || line.contains("XpsbShaderSize")) {
                        out.printf("MATCH function=0x%x name=%s line=%s%n",
                            f.getEntryPoint().getOffset() - delta, f.getName(), line.trim());
                    }
                }
            }
            out.println("FUNCTIONS_DECOMPILED=" + count);
        } finally {
            dec.dispose();
        }
        println("Xpsb static-buffer trace written to " + output);
    }
}
