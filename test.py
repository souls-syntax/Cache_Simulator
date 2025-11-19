
import subprocess
import csv

# CONFIG
trace_files = ["traces/long.trace", "traces/short.trace"]   # add your traces here
s_values = range(1, 9)   # tweak as needed
E_values = range(1, 9)
b_values = range(2, 7)

output_file = "results.csv"

def run_sim(trace, s, E, b):
    try:
        out = subprocess.check_output([
            "./cache_simulator",
            "-s", str(s),
            "-E", str(E),
            "-b", str(b),
            "-t", trace
        ], stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        return None

    text = out.decode()

    hits = misses = evictions = None

    for line in text.splitlines():
        line = line.strip()
        if line.startswith("Hits:"):
            hits = int(line.split()[1])
        elif line.startswith("Misses:"):
            misses = int(line.split()[1])
        elif line.startswith("Evictions:"):
            evictions = int(line.split()[1])

    if hits is None or misses is None or evictions is None:
        return None

    total = hits + misses
    hit_rate = hits / total if total > 0 else 0
    miss_rate = misses / total if total > 0 else 0

    return hits, misses, evictions, hit_rate, miss_rate


# WRITE CSV
with open(output_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["trace", "s", "E", "b", "hits", "misses", "evictions", "hit_rate", "miss_rate"])

    for trace in trace_files:
        for s in s_values:
            for E in E_values:
                for b in b_values:
                    result = run_sim(trace, s, E, b)
                    if result:
                        hits, misses, evictions, hit_rate, miss_rate = result
                        writer.writerow([
                            trace, s, E, b, hits, misses, evictions, hit_rate, miss_rate
                        ])


