import tkinter as tk
import random

class Particle:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.size = random.randint(5, 10)
        self.speed_x = random.uniform(-2, 2)
        self.speed_y = random.uniform(-2, 2)
        self.color = f'#{random.randint(0x010101, 0xFFFFFF):06x}'

    def update(self):
        self.x += self.speed_x
        self.y += self.speed_y

        # Bounce off walls
        if self.x < 0 or self.x > canvas_width:
            self.speed_x *= -1
        if self.y < 0 or self.y > canvas_height:
            self.speed_y *= -1

class ParticleSystem:
    def __init__(self):
        self.particles = []

    def create_particles(self, event):
        for _ in range(5):
            self.particles.append(Particle(event.x, event.y))

    def update(self):
        for particle in self.particles:
            particle.update()
            self.draw_particle(particle)
        
        # Remove particles that are too small
        self.particles = [p for p in self.particles if p.size > 0]

    def draw_particle(self, particle):
        x0 = particle.x - particle.size
        y0 = particle.y - particle.size
        x1 = particle.x + particle.size
        y1 = particle.y + particle.size
        canvas.create_oval(x0, y0, x1, y1, fill=particle.color, outline="")
        particle.size = max(0, particle.size - 0.1)  # Gradually reduce size

def animate():
    canvas.delete("all")
    particle_system.update()
    root.after(16, animate)

root = tk.Tk()
root.title("Interactive Particle System")
canvas_width = 800
canvas_height = 600
root.geometry(f"{canvas_width}x{canvas_height}")

canvas = tk.Canvas(root, width=canvas_width, height=canvas_height, bg='black')
canvas.pack()

particle_system = ParticleSystem()
canvas.bind("<Motion>", particle_system.create_particles)

animate()
root.mainloop()