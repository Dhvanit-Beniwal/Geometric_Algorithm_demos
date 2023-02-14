#include <SFML/Graphics.hpp>
#include <set>
#include <vector>
#include <iostream>

#define EPSILON 1e-7
#define CLOSE(a,b) (abs(a-b)<EPSILON)

auto cmp_dot = [](sf::CircleShape a, sf::CircleShape b) {
    if(!CLOSE(a.getPosition().x, b.getPosition().x)){
        return a.getPosition().x < b.getPosition().x;
    }else{
        return a.getPosition().y > b.getPosition().y;
    }
};

auto cmp_chain = [](std::vector<sf::Vertex>* chain1, std::vector<sf::Vertex>* chain2) {
    return chain1->back().position.y < chain2->back().position.y;
};

std::set< sf::CircleShape, decltype(cmp_dot) > points(cmp_dot);
std::set< std::vector<sf::Vertex>* , decltype(cmp_chain) > chains(cmp_chain);

// "dot" referes to a mathematical point but drawn with a thickness, represented as an SFML circle
// "vertex" refers to just the 1-pixel point, also an SFML vertex
// programmatically convenient to draw() chain with vertex-array but visually convenient to draw() points with dots

sf::CircleShape make_dot(float x, float y){
    float r = 2.f;
    sf::CircleShape point(r, 10);
    point.setPosition(x-r,y-r);
    point.setFillColor(sf::Color::White);
    point.setOutlineThickness(2.f);
    point.setOutlineColor(sf::Color::Blue);
    return point;
}
sf::Vertex dot_to_vertex(sf::CircleShape dot){
    float r = dot.getRadius();
    return sf::Vertex(sf::Vector2f(dot.getPosition().x+r, dot.getPosition().y+r), sf::Color::White);
}

void make_chains(){
    for(auto chain: chains){
        delete chain;
    }
    chains.clear();
    for(auto p: points){
        if(chains.empty()){
            chains.insert(new std::vector<sf::Vertex>{dot_to_vertex(p)});
            continue;
        }
        bool valid = false;
        for(auto it = chains.begin(); it!=chains.end(); ++it){
            auto& q = (*it)->back();
            valid = (p.getPosition().y < q.position.y); // don't need to check x coord since points are sorted acc to that already
            if(valid){
                auto chain = *it;
                chains.erase(it);
                chain->push_back(sf::Vertex(dot_to_vertex(p)));
                chains.insert(chain);
                break;
            }
        }
        if(!valid){
            chains.insert(new std::vector<sf::Vertex>{sf::Vertex(dot_to_vertex(p))});
        }
    }
    // for(auto chain : chains){std::cout << chain->size() << std::endl;}
}


int main(){      
    bool display_chains = true;
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 6;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "minimal partition into monotonically increasing chains", sf::Style::Default, settings);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed){window.close();}
            if (event.type == sf::Event::MouseButtonPressed){
                if(event.mouseButton.button == sf::Mouse::Left){
                    points.insert(make_dot(event.mouseButton.x, event.mouseButton.y));
                }else
                if(event.mouseButton.button == sf::Mouse::Right){
                    make_chains();
                }
            }
            if(event.mouseButton.button == sf::Mouse::Middle){
                points.clear();
                for(auto chain: chains){
                    delete chain;
                }
                chains.clear();
            }
            if (event.type == sf::Event::KeyPressed){
                if (event.key.code == sf::Keyboard::H){
                    display_chains = !display_chains;
                }
            }
        }

        window.clear(sf::Color::Black);
        for(auto p: points){window.draw(p);}
        if(display_chains){
            for(auto chain: chains){
                window.draw(&(*chain)[0], chain->size(), sf::LineStrip);
            }
        }
        window.display();
    }

    return 0;
}