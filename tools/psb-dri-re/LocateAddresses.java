// @category PSB
// Locate containing functions for selected ELF virtual addresses in the disposable project.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class LocateAddresses extends GhidraScript {
    public void run() throws Exception {
        long[] elfVas = {0x001cab00L, 0x001c6c08L, 0x001fb922L};
        for (long elfVa : elfVas) {
            Address address = toAddr(elfVa + 0x10000L);
            Function function = getFunctionContaining(address);
            if (function == null) {
                println(String.format("PSB_LOCATE 0x%08x none", elfVa));
            } else {
                long entryElfVa = function.getEntryPoint().getOffset() - 0x10000L;
                println(String.format("PSB_LOCATE 0x%08x 0x%08x %s", elfVa, entryElfVa, function.getName()));
            }
        }
    }
}
