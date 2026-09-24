// Static Ghidra export. Run only in a separate headless project.
// Decompiler output goes to /tmp and is deliberately not project documentation.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.SymbolIterator;
import ghidra.program.model.address.Address;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashSet;
import java.util.Set;

public class ExportPsb extends GhidraScript {
    private static final String OUT = "/home/gama/sgx535-gfx/docs/phase7/psb-dri-re/analysis";
    private static final String DECOMP = "/tmp/sgx535-psb-decompile";
    private long imageDelta;

    private String q(Object x) {
        String s = x == null ? "" : x.toString();
        return "\"" + s.replace("\"", "\"\"").replace("\n", " ").replace("\r", " ") + "\"";
    }

    private String elfVa(Address a) {
        long n = a.getOffset() - imageDelta;
        return String.format("0x%08x", n);
    }

    @Override
    public void run() throws Exception {
        SymbolIterator symbols = currentProgram.getSymbolTable().getSymbols("__driDriverExtensions");
        if (!symbols.hasNext()) throw new IllegalStateException("No __driDriverExtensions symbol");
        imageDelta = symbols.next().getAddress().getOffset() - 0x2bb1bcL;
        println("PSB_EXPORT image_delta=" + String.format("0x%x", imageDelta));
        File out = new File(OUT);
        out.mkdirs();
        File decomp = new File(DECOMP);
        decomp.mkdirs();
        PrintWriter functions = new PrintWriter(new FileWriter(new File(out, "functions.csv")));
        PrintWriter graph = new PrintWriter(new FileWriter(new File(out, "callgraph.csv")));
        PrintWriter imports = new PrintWriter(new FileWriter(new File(out, "import-callers.csv")));
        functions.println("elf_va,ghidra_va,name,body_bytes,is_thunk,calling_function_count,called_function_count");
        graph.println("caller_elf_va,callee_elf_va,caller_name,callee_name");
        imports.println("import_name,caller_elf_va,caller_ghidra_va,caller_name");
        Set<Long> interesting = new HashSet<Long>();
        // 23 DRI descriptor callbacks plus confirmed extension list reader.
        long[] callbacks = {0x21f8d,0x213a0,0x1ec1d,0x1ec67,0x21846,0x21790,
            0x21396,0x21757,0x219fa,0x211d1,
            0x21e0e,0x215e2,0x214e9,0x21c89,0x216e6,0x215aa,
            0x213ab,0x21456,0x2137d,0x2138b,0x21297,0x21278,0x2130f};
        for (long x : callbacks) interesting.add(x);
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        int total = 0, edges = 0, imported = 0, decompiled = 0;
        while (it.hasNext() && !monitor.isCancelled()) {
            Function f = it.next();
            total++;
            Set<Function> called = f.getCalledFunctions(monitor);
            Set<Function> callers = f.getCallingFunctions(monitor);
            functions.println(q(elfVa(f.getEntryPoint())) + "," + q(f.getEntryPoint()) + "," + q(f.getName()) + "," +
                f.getBody().getNumAddresses() + "," + f.isThunk() + "," + callers.size() + "," + called.size());
            for (Function c : called) {
                graph.println(q(elfVa(f.getEntryPoint())) + "," + q(elfVa(c.getEntryPoint())) + "," + q(f.getName()) + "," + q(c.getName()));
                edges++;
            }
            if ((f.isExternal() || f.isThunk()) && f.getEntryPoint().getOffset() - imageDelta < 0x20000L &&
                (f.getName().startsWith("drm") || f.getName().equals("ioctl") ||
                                   f.getName().equals("mmap") || f.getName().equals("munmap"))) {
                for (Function caller : callers) {
                    imports.println(q(f.getName()) + "," + q(elfVa(caller.getEntryPoint())) + "," +
                        q(caller.getEntryPoint()) + "," + q(caller.getName()));
                    imported++;
                    if (f.getName().startsWith("drmCommand")) interesting.add(caller.getEntryPoint().getOffset() - imageDelta);
                }
            }
        }
        functions.close(); graph.close(); imports.close();

        PrintWriter xrefs = new PrintWriter(new FileWriter(new File(out, "string-xrefs.csv")));
        xrefs.println("string_elf_va,ref_from_elf_va,function_elf_va,function_name,reference_type");
        int xrefCount = 0;
        try (BufferedReader reader = new BufferedReader(new FileReader(new File(out, "interesting-strings.csv")))) {
            reader.readLine();
            String line;
            while ((line = reader.readLine()) != null && !monitor.isCancelled()) {
                int comma = line.indexOf(',');
                if (comma < 0 || !line.startsWith("0x")) continue;
                long elf = Long.parseUnsignedLong(line.substring(2, comma), 16);
                ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(toAddr(elf + imageDelta));
                while (refs.hasNext()) {
                    Reference ref = refs.next();
                    Function parent = currentProgram.getFunctionManager().getFunctionContaining(ref.getFromAddress());
                    xrefs.println(q(String.format("0x%08x", elf)) + "," + q(elfVa(ref.getFromAddress())) + "," +
                        q(parent == null ? "" : elfVa(parent.getEntryPoint())) + "," +
                        q(parent == null ? "" : parent.getName()) + "," + q(ref.getReferenceType()));
                    xrefCount++;
                }
            }
        }
        xrefs.close();

        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        for (long elf : interesting) {
            if (monitor.isCancelled()) break;
            Address a = toAddr(elf + imageDelta);
            Function f = currentProgram.getFunctionManager().getFunctionContaining(a);
            if (f == null || f.isExternal()) continue;
            DecompileResults result = dec.decompileFunction(f, 30, monitor);
            if (!result.decompileCompleted() || result.getDecompiledFunction() == null) continue;
            File dest = new File(decomp, String.format("%08x.txt", elf));
            try (PrintWriter pw = new PrintWriter(new FileWriter(dest))) {
                pw.println("ELF VA " + String.format("0x%08x", elf) + " Ghidra " + a + " " + f.getName());
                pw.println(result.getDecompiledFunction().getC());
            }
            decompiled++;
        }
        dec.dispose();
        println("PSB_EXPORT functions=" + total + " edges=" + edges + " import_callers=" + imported +
                " string_xrefs=" + xrefCount + " decompiled=" + decompiled + " (decompiler files in /tmp)");
    }
}
