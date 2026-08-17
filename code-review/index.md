---
layout: default
title: Code Review
---

# Code Review

## Overview

For my code review, I selected my Animal Shelter Dashboard project from CS 340. The project uses Python and MongoDB to store and manage animal shelter records. I selected this project because it gave me a good opportunity to look back at code I had already written and think about how I could improve it rather than simply creating something new.

The original project was functional, but there were several areas where I could improve the organization, usability, and overall quality of the application. Reviewing my own code also helped me recognize that software development does not end when an application works. Code can usually be improved through better organization, clearer structure, improved functionality, and more thoughtful design decisions.

## Original Project

The Animal Shelter Dashboard was created to allow users to work with animal shelter information stored in a MongoDB database. The application uses Python to connect to the database and perform operations on the stored records.

The original project gave me experience working with MongoDB, Python, database connections, and CRUD operations. It also helped me understand how an application can interact with a database instead of relying on information that is hard-coded into the program.

One of the most important parts of the project was the `AnimalShelter` class. The class handles the connection to MongoDB and provides methods for creating, reading, updating, and deleting records. This allowed the database functionality to be separated from the rest of the application and made it easier to work with the stored data.

## Code Review

When I reviewed the original project, I looked at both what was working and what could be improved. The project successfully connected to MongoDB and performed the required database operations, but I identified several areas where the application could be made more useful and easier to maintain.

One area I wanted to improve was the way users could search and filter information. The original project provided the basic functionality needed to retrieve records, but there was room to make the process more flexible and easier for a user to work with. Improving the filtering and search functionality would make it easier to find specific animals without requiring the user to understand how the database itself is organized.

I also looked at how the information was displayed. A database can contain the correct information and still be difficult to use if the results are not presented clearly. Improving the way records are displayed would make the application easier to understand and would give users a better experience when working with the information.

Another area I considered was the organization of the database operations. Keeping database interactions organized is important because the application may need to perform many queries as the amount of information grows. A more organized structure can also make the code easier to maintain and modify later.

## Improvements

The changes I focused on were intended to make the project more useful without changing the purpose of the original application.

One improvement was making the search and filtering process more flexible. Instead of relying on a limited search approach, the application could provide users with more ways to narrow down the records they are looking for.

I also focused on improving how the results are presented. Making the output easier to read helps separate the database functionality from the user experience. The goal was not just to retrieve the correct information but to make that information useful to someone actually using the application.

Another improvement was making the code easier to understand and maintain. Organizing related database functionality together makes it easier to find where changes need to be made. This is especially important in software that may eventually be worked on by multiple developers.

## What I Learned

The code review process taught me that looking back at older work can be just as useful as creating something new. When I originally completed the Animal Shelter Dashboard, my main goal was to get the required functionality working. After gaining more experience throughout the program, I was able to look at the same project differently and recognize areas where I could improve it.

This also showed me how much my approach to software development has changed. I am more aware now of things like organization, usability, maintainability, and scalability. I do not only think about whether the code works. I also think about whether another developer could understand it, whether a user could easily interact with it, and whether the application could be expanded later.

The code review also reinforced the idea that software development is an ongoing process. There is usually more that can be improved after the first working version of an application. Being able to identify those improvements and make changes is an important part of becoming a better software engineer.

## Video Presentation

The following video provides an overview of my code review and the changes I made to the project.

[Watch My Code Review Presentation](https://www.youtube.com/watch?v=hp-_c32tOC8)
