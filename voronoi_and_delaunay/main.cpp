#include <cstdio>

#include <SFML/Graphics.hpp>
#include <set>
#include <vector>
#include <iostream>
#include <cmath>
#include <unordered_set>

#include "algorithm.hpp"

int main(int argc, char**argv){
    
    int window_x = 1000;
    int window_y = 750;
    
    if(argc > 1){ // read a set of points from std::cin (or a file dumped into stdin)
        read_from_stdin(window_x, window_y);
    }
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(window_x, window_y), "Voronoi Diagram", sf::Style::Default, settings);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed){window.close();}
            if (event.type == sf::Event::MouseButtonPressed){
                if(event.mouseButton.button == sf::Mouse::Left){
                    points.push_back(Point(event.mouseButton.x, event.mouseButton.y));
                }else
                if(event.mouseButton.button == sf::Mouse::Right){
                    voronoi();
                }
            }
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::R){
                    points.clear();
                    Q.clear();
                    T.clear();
                    D.clear();
                }
                if(event.key.code == sf::Keyboard::D){
                    show_delaunay = !show_delaunay;
                }
                if(event.key.code == sf::Keyboard::V){
                    show_voronoi = !show_voronoi;
                }
            }
        }

        window.clear(sf::Color::Black);
        
        for(auto& p: points){window.draw(p);}
        for(auto& l: D){window.draw(l);}
        
        window.display();
    }

    return 0;
}
