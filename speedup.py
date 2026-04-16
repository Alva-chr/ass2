import numpy as np
import matplotlib.pyplot as plt

#processes
p = [1, 2, 4, 8]

#speedup
data_strong = []

data_weak = []

ideal = p #ideal speedup

plt.figure(figsize=(10, 6))

# Plot speedup strong
plt.plot(p, data_strong, marker='o', label='Strong scalability')

# Plot speedup weak
plt.plot(p, data_weak, marker='o', label='Weak scalability')

# Plot ideal speedup
plt.plot(p, ideal, 'k--', label='Ideal Speedup')

plt.xlabel('Number of Processes (p)')
plt.ylabel('Speedup')

#Title strong
plt.title('Speedup for n = 8000000  vs ideal speedup')

#Title weak
plt.title('Speedup for varying n vs ideal speedup')

plt.xticks(p)
plt.grid(True, which="both", ls="--")
plt.legend()
plt.tight_layout()
plt.show()