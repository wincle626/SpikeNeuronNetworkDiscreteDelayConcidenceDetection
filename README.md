# SpikeNeuronDCD

Discrete Delay Concidence Detection (DCD) Spiking Neural Network Modelling Framework

This is a small framework for modelling transmission delay discrete (on/off) spiking processes with various network topology options, including random, 2D, and Small World.

These networks can exhibit highly complex dynamics and potentially related to cortical or other coincidence detection dynamics in nervous systems.

Compile using: gcc arbn.c automaton_viewer.c -o arbn -lcurses -lgsl -I/usr/local/include/gsl

Related paper: Jeanson, F., White, T. (2013). Dynamic Memory for Robot Control using Delay-based Coincidence Detection Neurones. In Artificial neural networks and machine learning, ICANN, Springer, pages 280–287. 
