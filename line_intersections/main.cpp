// #include <iostream>

#include <SFML/Graphics.hpp>
#include <set>
#include <vector>
#include <cmath>
#include <limits>

#include "algorithm.hpp"

int main(){      
    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "intersections of line segments", sf::Style::Default, settings);
    
    double x,y; // buffer for user input of line segments by clicking twice
    bool flag = false;
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed){window.close();}
            if (event.type == sf::Event::MouseButtonPressed){
                if(event.mouseButton.button == sf::Mouse::Left){
                    if(!flag){
                        x = event.mouseButton.x;
                        y = event.mouseButton.y;
                    }else{
                        lines.push_back(Line(x,y,event.mouseButton.x, event.mouseButton.y));
                        if(y < sweep_line_y){sweep_line_y = y;}
                        if(event.mouseButton.y < sweep_line_y){sweep_line_y = event.mouseButton.y;}
                    }
                    flag = !flag;
                }else
                if(event.mouseButton.button == sf::Mouse::Right){
                    find_intersections();
                }
            }
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::R){
                    lines.clear();
                    intersections.clear();
                    flag = false; 
                }
            }
        }

        window.clear(sf::Color::Black);
        for(auto l: lines){window.draw(l);}
        for(auto i: intersections){window.draw(i);}
        window.display();
    }
    return 0;
}