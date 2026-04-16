import numpy as np
import matplotlib.pyplot as plt

#processes
p = [1, 2, 4, 8]

#speedup
data_strong = [1, 1.87, 2.0297, 2.0633]

data_weak = [1, 0.9078, 0.9131, 0.2086]

strong_ideal = p #ideal speedup for strong scaling

weak_ideal = [1, 1, 1, 1] #ideal speedup for weak scaling

plt.figure(figsize=(10, 6))

# Plot speedup strong
plt.plot(p, data_strong, marker='o', label='Strong scalability')

# Plot speedup weak
#plt.plot(p, data_weak, marker='o', label='Weak scalability')

# Plot ideal speedup strong
plt.plot(p, strong_ideal, 'k--', label='Ideal Speedup')

# Plot ideal speedup weak
#plt.plot(p, weak_ideal, 'k--', label='Ideal Speedup')

plt.xlabel('Number of Processes (p)')
plt.ylabel('Speedup')

#Title strong
plt.title('Speedup for n = 8000000  vs ideal speedup')

#Title weak
#plt.title('Speedup for varying n vs ideal speedup')

plt.xticks(p)
plt.grid(True, which="both", ls="--")
plt.legend()
plt.tight_layout()
plt.show()