#include <SFML/Graphics.hpp>
#include <set>
#include <vector>
#include <iostream>

#define EPSILON 1e-7
#define CLOSE(a,b) (abs(a-b)<EPSILON)

// "dot" referes to a mathematical point but drawn with a thickness, represented as an SFML circle
// "vertex" refers to just the 1-pixel point, also an SFML vertex
// programmatically convenient to draw() chain with vertex-array but visually convenient to draw() points with dots

class Point : public sf::Drawable{
        sf::CircleShape dot;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            target.draw(dot, states);
        }
    public:
        sf::Vertex vertex;
        Point(float x, float y){
            float r = 2.f;
            dot = sf::CircleShape(r, 10);
            dot.setPosition(x-r,y-r);
            dot.setFillColor(sf::Color::White);
            dot.setOutlineThickness(2.f);
            dot.setOutlineColor(sf::Color::Blue);
            vertex = sf::Vertex(sf::Vector2f(x, y), sf::Color::White);
        }
        bool operator< (const Point& rhs) const{
            if(!CLOSE(vertex.position.x, rhs.vertex.position.x)){
                return vertex.position.x < rhs.vertex.position.x;
            }else{
                return vertex.position.y > rhs.vertex.position.y;
            }
        }
};

class Chain : public sf::Drawable{
        std::vector<sf::Vertex> varray;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            target.draw(&varray[0], varray.size(), sf::LineStrip, states);
        }
    public:
        Chain(sf::Vertex first){
            varray.push_back(first);
        }
        void add(Point& p) {
            varray.push_back(p.vertex);
        }
        bool operator< (const Chain& rhs) const{                        // chain < chain
            return varray.back().position.y < rhs.varray.back().position.y;
        }
        bool operator< (const Point& rhs) const{                        // chain < point
            return varray.back().position.y < rhs.vertex.position.y;
        }
        friend bool operator< (const Point& lhs, const Chain& rhs) {    // point < chain
            return lhs.vertex.position.y < rhs.varray.back().position.y;
        }
};

std::set< Point > points;
std::set< Chain, std::less<> > chains;

void make_chains(){
    chains.clear();
    for(auto p: points){
        if(chains.empty()){
            chains.insert(Chain(p.vertex));
            continue;
        }

        auto it = chains.upper_bound(p);    // highest chain able to accept p
        if(it == chains.end()){
            chains.insert(Chain(p.vertex));
        }else{
            auto chain = *it;
            chain.add(p);   // should update in-place but std::set doesn't allow it since it doesn't know that in this case order never changes on updating a chain
            chains.erase(it); chains.insert(chain); // therefore an inefficiency (should only need the one 'log_n' traversal with call to upper_bound() )
        }
    }
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
                    points.insert(Point(event.mouseButton.x, event.mouseButton.y));
                }else
                if(event.mouseButton.button == sf::Mouse::Right){
                    make_chains();
                }
            }
            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::R){
                    points.clear();
                    chains.clear();
                }else
                if (event.key.code == sf::Keyboard::H){
                    display_chains = !display_chains;
                }
            }
        }

        window.clear(sf::Color::Black);
        
        for(auto p: points){window.draw(p);}
        
        if(display_chains){
        for(auto c: chains){window.draw(c);}
        }
        
        window.display();
    }

    return 0;
}