// Read-only Ghidra XREF report for the frozen triangle entry and program-selection dependencies.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.*;

public class TraceFrozenDraw extends GhidraScript {
    public void run() throws Exception {
        SymbolIterator roots = currentProgram.getSymbolTable().getSymbols("__driDriverExtensions");
        if (!roots.hasNext()) throw new IllegalStateException("DRI root missing");
        long delta = roots.next().getAddress().getOffset() - 0x2bb1bcL;
        long[] targets = {0x52fcc, 0x2bb280, 0x33490, 0x3503d, 0x40355, 0x39ac4};
        try (PrintWriter out = new PrintWriter(new FileWriter("/tmp/sgx535-psb-frozen-xrefs.txt"))) {
            for (long va : targets) {
                Address to = toAddr(va + delta);
                out.printf("TARGET 0x%x%n", va);
                ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(to);
                while (refs.hasNext()) {
                    Reference ref = refs.next();
                    Address from = ref.getFromAddress();
                    Function owner = currentProgram.getFunctionManager().getFunctionContaining(from);
                    out.printf("  source=0x%x type=%s owner=%s%n", from.getOffset() - delta,
                        ref.getReferenceType(), owner == null ? "NONE" : owner.getName());
                }
            }
        }
        println("Wrote /tmp/sgx535-psb-frozen-xrefs.txt");
    }
}
