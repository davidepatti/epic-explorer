from m5 import *

class M5E_Cluster(SimObject):
    type = 'M5E_Cluster'
    cluster_id = Param.Int("cluster ID")
    icache = Param.BaseCache("L1 D-cache")
    dcache = Param.BaseCache("L1 D-cache")
    main_mem = Param.FunctionalMemory("hierarchical memory")
    max_loads = Param.Counter("number of loads to execute")
    memory_size = Param.Int(65536, "memory size")
    percent_reads = Param.Percent(65, "target read percentage")
    progress_interval = Param.Counter(1000000,
        "progress report interval (in accesses)")
    trace_addr = Param.Addr(0, "address to trace")
