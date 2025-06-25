

# Proyecto de Comparación y Evaluación del Algoritmo Cuckoo Search (CSA)

Este proyecto tiene como objetivo evaluar el rendimiento del algoritmo Cuckoo Search aplicado al problema de la mochila, comparándolo con una estrategia de fuerza bruta. Se incluyen cuatro módulos principales que abordan desde la implementación básica hasta el análisis estadístico y generación de resultados automáticos.



## Archivos incluidos

### `moi.cpp`
**Descripción:**  
Este archivo contiene la implementación base del algoritmo **Cuckoo Search (CSA)** y una comparación directa con el enfoque **de fuerza bruta**.  
Permite observar el comportamiento del algoritmo metaheurístico frente a un algoritmo exacto en términos de eficiencia y calidad de solución.

**Contenido principal:**
- Planteamiento del problema de la mochila.
- Solución encontrada
- Implementación completa del CSA.
- Comparación con fuerza bruta (individual y 10000 iteraciones).
- Visualización de solución y fitness.

---

### `moi2.cpp`
**Descripción:**  
Archivo complementario que **desglosa paso a paso** el funcionamiento del CSA. Incluye mensajes detallados por consola para observar la evolución del fitness y la selección de soluciones.

**Contenido principal:**
- Explicación detallada de cada iteración.
- Registro del reemplazo de soluciones.
- Visualización de los nidos y sus cambios.
- Código utilizado para el vídeo.

---

### `moi3.cpp`
**Descripción:**  
Este script automatiza la ejecución del algoritmo usando un archivo de entrada `config.txt` y genera un archivo de salida `resultados.csv` con los resultados.  
Se usa principalmente para realizar análisis experimentales variando parámetros como:
- Número de ítems.
- Número de nidos.
- Número de iteraciones.

**Archivo `config.txt`:**

```txt
# Formato de entrada
n_items=1000
n_nests=15
max_iter=500
repeticiones=10
```

**Archivo generado:**

* `resultados.csv`: contiene los valores de fitness, tiempo de ejecución y desviación estándar.

**Archivo `graficos.ipynb`:**

Archivo Jupyter notebook para la generación de gráficos en base a resultados.


---

### `moi4.cpp`

**Descripción:**
Este archivo está dedicado al **análisis estadístico de las métricas del CSA** a lo largo de múltiples ejecuciones (por ejemplo, 10,000). Evalúa:

* Fitness promedio.
* Desviación estándar del fitness.
* Error absoluto y relativo frente al óptimo.
* Tiempo promedio y desviación estándar del tiempo.
* Número de veces que se alcanzó o no el óptimo.

**Ideal para:**
Evaluar la **calidad de la solución y estabilidad del algoritmo**.

---

## Requisitos

* Compilador C++ compatible con C++11 o superior.
* `g++` o equivalente (Visual Studio, Clang).
* Opcional: herramientas para graficar resultados (`Python + matplotlib` si se desea visualización).

---


## Integrantes del grupo

- David Mamani
- Renzo Arroyo
- Piero Rozas
- Alvaro Sanchez

