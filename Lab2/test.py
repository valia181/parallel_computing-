import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("result.csv")

plt.figure(figsize=(10, 6))

num_threads = df['Num_Threads'].iloc[0]

plt.plot(df['Array_Size'], df['Seq_Time'], marker='o', label='Sequence', linewidth=2, color='red')
plt.plot(df['Array_Size'], df['Mutex_Time'], marker='s', label='Mutex', linewidth=2, color='blue')
plt.plot(df['Array_Size'], df['CAS_Time'], marker='^', label='CAS', linewidth=2, color='green')

plt.xscale('log')
plt.yscale('log')

plt.title(f"Algorithm performance\n(Threads: {num_threads})",
          fontsize=14, fontweight='bold')
plt.xlabel("Array size", fontsize=12)
plt.ylabel("Execution time, ms", fontsize=12)

plt.legend(fontsize=11)
plt.grid(True, which="both", linestyle='--', alpha=0.6)

plt.tight_layout()

plt.show()