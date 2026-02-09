import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv", header = None, names = ["Size", "Threads", "Time"])

unique_sizes = df['Size'].unique()

for size in unique_sizes:
    subset = df[df['Size'] == size]

    plt.figure(figsize = (10, 6))

    plt.plot(subset['Threads'], subset['Time'], marker = 'o', linewidth = 2, color = 'b')

    plt.title(f"Розмірність матриці: {size}", fontsize = 16, fontweight = 'bold')

    plt.xlabel("Кількість потоків", fontsize = 12)
    plt.ylabel("Час, мс", fontsize = 12)

    plt.grid(True, linestyle = '--', alpha = 0.7)

    plt.xticks(subset['Threads'].unique())

    plt.show()