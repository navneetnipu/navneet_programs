import matplotlib.pyplot as mplt
import numpy as np
A=input("enter the value of amplitide for the sinusoidal wave:")
print("\n")
T=input("enter the time period for the wave in seconds:")
print("\n")
l=input("enter the wave length for teh wave in meters:")
print("\n")
x=input("enter the x cordinate range value:")
print("\n")
t=input("enter the value for time in seconds:")
print("\n")
#f(i,j)=A*np.sin(2*pi*(1/T)*i)*np.cos(2*pi*(1/T)*j)
f = lambda i,j: A*np.sin(2*np.pi*(1/T)*i)*np.cos(2*np.pi*(1/T)*j)
print("Hello");
for i in range(-x,x,1):
    for j in range(-t,t,1):
        print(f(i,j))
    print("\n")

