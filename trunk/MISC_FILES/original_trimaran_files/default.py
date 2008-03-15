
from M5E_Cluster import *


class IL1Cache(BaseCache):
	size = '4kB'
	assoc = 2
	block_size = 16
	mshrs = 32
	tgts_per_mshr = 16
	latency = 1 * Parent.clock.period
	protocol = Parent.busProtocol

class DL1Cache(BaseCache):
	size = '4kB'
	assoc = 2
	block_size = 16
	mshrs = 32
	tgts_per_mshr = 16
	latency = 1 * Parent.clock.period
	protocol = Parent.busProtocol

class L2Cache(BaseCache):
	size = '512kB'
	assoc = 4
	block_size = 64
	mshrs = 32
	tgts_per_mshr = 16
	latency = 10 * Parent.clock.period

class MyBaseMemory(BaseMemory):
	latency = 100 * Parent.clock.period
	snarf_updates = False
	do_writes = True

class MyCluster(M5E_Cluster):
	icache = IL1Cache(out_bus=Parent.toL2Bus)
	dcache = DL1Cache(out_bus=Parent.toL2Bus)
	main_mem = Parent.mainMem
	max_loads = 5000000

class MyRoot(Root):
    hier = HierParams(do_data=True, do_events=True)
    toMemBus = Bus(width=128, clock=1 * Parent.clock.period)
    toL2Bus = Bus(width=64, clock=1 * Parent.clock.period)
    L2 = L2Cache(in_bus=Parent.toL2Bus, out_bus=Parent.toMemBus)
    SDRAM = MyBaseMemory(in_bus=Parent.toMemBus)
    mainMem = MainMemory()
    cluster = MyCluster()
    busProtocol = CoherenceProtocol(protocol='moesi')

root = MyRoot()
#root.trace.flags = "Bus Cache Event"
root.cluster = [ MyCluster(cluster_id=i) for i in xrange(4) ] 

