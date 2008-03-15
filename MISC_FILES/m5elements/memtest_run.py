##############################################################################
# WARNING:
#     this file is old and hasn't been tested in quite a while
#     although I think I just updated the syntax to be
#     compatible with M5 v1.1
#     <lieberm 01/20/2006>
#
##############################################################################

class DL1Cache(BaseCache):
    size = '64kB'
    assoc = 2
    block_size = 64
    tgts_per_mshr = 16
    latency = 4 * Parent.clock.period
    mshrs = 32

class L2Cache(BaseCache):
    size = '64kB'
    assoc = 4
    block_size = 64
    tgts_per_mshr = 16
    latency = 10 * Parent.clock.period
    mshrs = 32
    do_copy = True

class MyRoot(Root):
    hier = HierParams(do_data=True, do_events=True)
    toL2Bus = Bus(width=64, clock=2 * Parent.clock.period)
    toMemBus = Bus(width=16, clock=4 * Parent.clock.period)
    toMemBusSlow = Bus(width=16, clock=8 * Parent.clock.period)
    DL1 = DL1Cache(in_bus=NULL, out_bus=Parent.toL2Bus)
    L2 = L2Cache(in_bus=Parent.toL2Bus, out_bus=Parent.toMemBus)
    busBridge = BusBridge(in_bus=Parent.toMemBus, out_bus=Parent.toMemBusSlow,
	max_buffer=16)
    SDRAM = BaseMemory(in_bus=Parent.toMemBusSlow, 
	latency=100 * Parent.clock.period, do_writes=True)
    mainMem = MainMemory()
    checkMem = MainMemory()
    test = MemTest(cache=Parent.DL1, main_mem=Parent.mainMem,
	check_mem=Parent.checkMem, max_loads=5000000)

root = MyRoot()
