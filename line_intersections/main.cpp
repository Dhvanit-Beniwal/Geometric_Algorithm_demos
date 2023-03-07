#include <SFML/Graphics.hpp>
#include <set>
#include <vector>
#include <iostream>

#include "algorithm.cpp"

std::vector< sf::CircleShape > sf_intersections;
std::vector< sf::Vertex > sf_lines;

sf::CircleShape make_dot(float x, float y){
    float r = 2.f;
    sf::CircleShape point(r, 10);
    point.setPosition(x-r,y-r);
    point.setFillColor(sf::Color::White);
    point.setOutlineThickness(8.f);
    point.setOutlineColor(sf::Color::Blue);
    return point;
}
sf::Vertex make_vertex(float x, float y){
    return sf::Vertex(sf::Vector2f(x, y), sf::Color::White);
}
sf::Vertex dot_to_vertex(sf::CircleShape dot){
    float r = dot.getRadius();
    return sf::Vertex(sf::Vector2f(dot.getPosition().x+r, dot.getPosition().y+r), sf::Color::White);
}

int main(){      
    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "intersections of line segments", sf::Style::Default, settings);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed){window.close();}
            if (event.type == sf::Event::MouseButtonPressed){
                if(event.mouseButton.button == sf::Mouse::Left){
                    sf_lines.push_back(make_vertex(event.mouseButton.x, event.mouseButton.y));
                }else
                if(event.mouseButton.button == sf::Mouse::Right){
                    auto& output = main_algo(sf_lines); // algorithm.cpp
                    for(auto p : output){
                        sf_intersections.push_back(make_dot(p.x, p.y));
                    }
                    output.clear(); // reference to a global var in algorithm.cpp
                }
            }
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::R){
                    sf_lines.clear();
                    sf_intersections.clear();    
                }
            }
        }

        window.clear(sf::Color::Black);
        window.draw(&sf_lines[0], sf_lines.size(), sf::Lines);
        for(auto p: sf_intersections){
            window.draw(p);
        }
        window.display();
    }
    return 0;
}