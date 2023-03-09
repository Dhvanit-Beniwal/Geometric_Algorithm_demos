// #include <iostream>

#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include <cmath>

#include "algorithm.hpp"

std::vector< Point > points;
std::vector< sf::Vertex > hull;

void make_hull(){
    std::sort(points.begin(), points.end());
    // hull = grahamScan(points.begin(), points.end());
    int n = points.size();
    int t = 0;
    while(t<n){
        int m = 1<<(1<<t); // 2^(2^t)
        if(m>n){m = n;}
        auto result = chan_algo(points, m);
        if(!result.empty()){
            hull = result;
            return;
        }
        ++t;
    }
}

int main(){      
    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "convex hull", sf::Style::Default, settings);
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
                    make_hull();
                }
            }
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::R){
                    points.clear();
                    hull.clear();
                }
            }
        }

        window.clear(sf::Color::Black);
        for(auto p: points){window.draw(p);}
        window.draw(&hull[0], hull.size(), sf::LineStrip);
        window.display();
    }

    return 0;
}