###Benjamin Daniel Resubmission for COMP3015 Coursework 2

#How does the user interact with your executable? How do you open and control the software you wrote (exe file)? 

Opening the executable found in folder "x64/release" shows a model of a cat with Shadows activated. Pressing 1 will deactivate and 2 will activate a toon shaded cat model, and Pressing 3 and 4 will do the same but for a particle system.

#How does the program code work? How do the classes and functions fit together and who does what? 

It uses a cat obj and a cat material. The light moves around the scene affecting the middle cat. The shadow created by this will affect the toon shaded cat next to it.
During the first pass of the render I render the base models of everything using the RenderProg, as well as draw the cell shaded cat, this is so the shadow will affect it in the next passes
In pass two and three i create the shadows by getting the scene ready to have the shadows applied to it in the third pass.

#What makes your shader program special and how does it compare to similar things? (Where did you get the idea from? What did you start with? How did you make yours unique? Did you start with a given shader/project?) 

It uses a particle system, shadows and a toon shader together to show what OpenGL has the capabilities of doing. With this knowledge I will be able to create games using my previous unit from scratch. 
I used the tutorials gave to use throughout the unit, change and adding them together to use the model I want and to put the together.

#A Link to the unlisted YouTube Video
https://www.youtube.com/watch?v=EOEZ83-tGcM
