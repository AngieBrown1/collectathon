A place to write your findings and plans

## Understanding

1. Angie - Change the speed of the player by adjusting the speed value
2. K - Change the backdrop by using set_backdrop and adding the backdrop library
3. Change the location of the player's starting position
    - Create constexpr variables PLAYER_START_X, PLAYER_START_Y, TREASURE_START_X, TREASURE_START_Y
    - Assign different values for those to change the starting position 
    - Add these variables in place of the original values in player, treasure initialization
4. Make it so when "start" is hit, the player and attributes restart
    - Within the while loop, check if start has been pressed
        - If it has been pressed, reset the player location
        - Set the player score to 0
        - Reset the player boost count
        - Set boosting to false
        - Reset treasure location
5. Add screen wraparound
    - Within the loop, check if the player goes out of bounds on both axisies, and then inverse the player position if they have crossd that boundary
6. Add a speed boosting feature
    - Initialize boost, boost_count, and boost_speed
    - If A has been pressed, there are boost counts left, and we aren't currently boosting
        - Set boosting equal to true
        - Add a value to the boost timer
        - Reduce the boost count
        - If we are currently boosting
            - Reduce the duration by 1 (If the framerrate is 60fps, then we can assume that 60 frame updates ~= 1 second)
            - If our boosting duration value is less than or equal to zero
            - Set boosting to false
    - Within our if statements to move the player, check if we are boosting, and either use the regualr speed or boosting speed. 

## Planning required changes

## Brainstorming game ideas

## Plan for implementing game

