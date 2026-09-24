// Read-only Ghidra XREF report for the unresolved historical draw/finalize dispatch.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.*;

public class TraceDrawDispatch extends GhidraScript {
    public void run() throws Exception {
        SymbolIterator roots = currentProgram.getSymbolTable().getSymbols("__driDriverExtensions");
        if (!roots.hasNext()) throw new IllegalStateException("DRI root missing");
        long delta = roots.next().getAddress().getOffset() - 0x2bb1bcL;
        long[] targets = {0x2673d, 0x26896, 0x26a0f, 0x2a39a, 0x37b51};
        try (PrintWriter out = new PrintWriter(new FileWriter("/tmp/sgx535-psb-draw-xrefs.txt"))) {
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
        println("Wrote /tmp/sgx535-psb-draw-xrefs.txt");
    }
}
