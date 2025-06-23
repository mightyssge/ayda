import numpy as np
from math import gamma

#define objective function
def objective_Fun(x):
    return np.sum(x**2)

#initialization of the population
def initial_Population(pop_size, dim):
    return np.random.uniform(-5, 5, (pop_size, dim))

#calculate levy flight
def levy_flight(beta):
    sigma = (gamma(1 + beta) * np.sin(np.pi * beta / 2) / (gamma((1 + beta) / 2) * beta * (2 ** ((beta - 1) / 2)))) ** (1 / beta)
    u = np.random.normal(0, sigma)
    v = np.random.normal(0, 1)
    step = u / (np.abs(v) ** (1 / beta))
    return step

#apply cuckoo search algorithm
def CS(Obj_Fun, Pop_Size = 50, Dim = 2 , MaxT = 100 , pa = 0.25):
    population = initial_Population(Pop_Size, Dim)
    fitness = np.array([Obj_Fun(nest) for nest in population])
    best_solution = None
    best_fitness = np.inf


    for i in range(MaxT):
        new_population = np.empty_like(population)

        for j,nest in enumerate(population):
            step_size = levy_flight(1.5)
            step_direction = np.random.uniform(-1, 1, Dim)
            new_nest = nest + step_size * step_direction
            new_population[j] = new_nest

            # Ensure new nest is within bounds
            new_nest = np.clip(new_population[j], -5, 5)

            new_fitness = np.array([Obj_Fun(new_nest) for nest in new_population])

            replace_soln = np.where(new_fitness < fitness)[0]
            population[replace_soln] = new_population[replace_soln]
            fitness[replace_soln] = new_fitness[replace_soln]
            
            min_idx = np.argmin(fitness)
            if fitness[min_idx] < best_fitness:
                best_fitness = fitness[min_idx]
                best_solution = population[min_idx].copy()

            abandon_egg = int(pa * Pop_Size)
            abandon_soln = np.random.choice(Pop_Size, size=abandon_egg, replace=False)
            population[abandon_soln] = initial_Population(abandon_egg, Dim)
            fitness[abandon_soln] = np.array([Obj_Fun(nest) for nest in population[abandon_soln]])

            print(f"Iteration {i+1}/{MaxT}, Best Fitness: {best_fitness}, Best Solution: {best_solution}")
        
    return best_solution, best_fitness


best_solution, best_fitness = CS(objective_Fun)
print(f"Best Solution: {best_solution}, Best Fitness: {best_fitness}")
    