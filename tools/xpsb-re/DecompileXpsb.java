// Static inspection in a disposable Ghidra project. Decompiled text stays in /tmp.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SymbolIterator;
import java.io.*;

public class DecompileXpsb extends GhidraScript {
    public void run() throws Exception {
        SymbolIterator si = currentProgram.getSymbolTable().getSymbols("XpsbInit");
        if (!si.hasNext()) throw new IllegalStateException("XpsbInit missing");
        long delta = si.next().getAddress().getOffset() - 0x2690L;
        File out = new File("/tmp/sgx535-xpsb-decompile"); out.mkdirs();
        DecompInterface dec = new DecompInterface(); dec.openProgram(currentProgram);
        int ok=0, missing=0, failed=0;
        try (BufferedReader br = new BufferedReader(new FileReader(
                "/home/gama/sgx535-gfx/tools/xpsb-re/targets.txt"))) {
            String line;
            while ((line=br.readLine())!=null && !monitor.isCancelled()) {
                line=line.trim(); if (line.isEmpty() || line.startsWith("#")) continue;
                String hex=line.split("\\s+",2)[0].replace("0x","");
                long va=Long.parseUnsignedLong(hex,16);
                Function f=currentProgram.getFunctionManager().getFunctionAt(toAddr(va+delta));
                if (f==null) f=currentProgram.getFunctionManager().getFunctionContaining(toAddr(va+delta));
                if (f==null || f.isExternal()) { missing++; println("XPSB missing 0x"+hex); continue; }
                File dest=new File(out,String.format("%08x.txt",va));
                if (dest.exists()) { ok++; continue; }
                DecompileResults result=dec.decompileFunction(f,30,monitor);
                if (!result.decompileCompleted() || result.getDecompiledFunction()==null) { failed++; continue; }
                try (PrintWriter pw=new PrintWriter(new FileWriter(dest))) {
                    pw.println("ELF VA 0x"+hex+" Ghidra "+f.getEntryPoint()+" "+f.getName());
                    pw.println(result.getDecompiledFunction().getC());
                }
                ok++; println("XPSB wrote 0x"+hex);
            }
        }
        dec.dispose(); println("XPSB complete="+ok+" missing="+missing+" failed="+failed);
    }
}
