#define INF std::numeric_limits<long double>::max()
#define EPSILON 1e-6
#define IS_ZERO(a) (a<EPSILON && a>-EPSILON)
#define CLOSE(a,b) (IS_ZERO(a-b))

long double sweep_line_y = INF;    // y-coordinate of the sweep line

class Point{    // Event
    public:
        long double x;
        long double y;
        // std::vector<Line> U; // Event is the key for this value in the std::map Q, so that we may modify U without erase-inserting Event
        Point(){}
        Point(const sf::Vertex& point){
            x = point.position.x;
            y = point.position.y;
        }
        Point(long double x, long double y){
            this->x = x;
            this->y = y;
        }
        bool operator<(const Point &rhs)const{
            if(!CLOSE(y,rhs.y)){
                return (y < rhs.y);   // sfml y-axis inverted
            }
            if(!CLOSE(x,rhs.x)){
                return (x < rhs.x);
            }
            return false;   // if CLOSE(lhs,rhs) then both lhs<rhs and rhs<lhs must return false.
        }
        // void print() const {    // debug
        //     std::cout << '(' << x << ", " << y << ")" << std::endl;
        //     return;
        // }
};

class Line : public sf::Drawable{
        sf::Vertex sf_line[2];
        Point upper, lower;
        
        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            target.draw(sf_line, 2, sf::Lines, states);
        }

        long double proj() const { // (lower-upper) -> (x/sqrt(x*x+y*y))
            long double x = lower.x - upper.x;
            long double y = lower.y - upper.y;
            return x/sqrt(x*x + y*y);
        }

        bool angle_cmp(const Line& rhs) const {            
            long double proj_a = this->proj();
            long double proj_b = rhs.proj();
            if(!CLOSE(proj_a, proj_b)){
                return (proj_a < proj_b);
            }else{  // when lines overlap, not sure about this. arbirtary?
                if(!CLOSE(lower.y, rhs.lower.y)){return lower.y < rhs.lower.y;}else
                if(!CLOSE(upper.y, rhs.lower.y)){return upper.y < rhs.upper.y;}
                else{return false;}
            }
        }

    public:
        Line(long double x1, long double y1, long double x2, long double y2){
            sf_line[0] = sf::Vertex(sf::Vector2f(x1, y1), sf::Color::White);
            sf_line[1] = sf::Vertex(sf::Vector2f(x2, y2), sf::Color::White);
            Point pa(x1,y1);
            Point pb(x2,y2);
            if( pa < pb ){
                upper = pa;
                lower = pb;
            }else{
                lower = pa;
                upper = pb;
            }
        }
        long double x_intercept(const long double& y) const { // assumed that Line segment instersects
            long double ydiff = lower.y - upper.y;
            if (CLOSE(ydiff, 0)){return upper.x;}
            long double xdiff = lower.x - upper.x;
            return (xdiff/ydiff)*(y-lower.y) + lower.x;
        }
        bool operator< (const Line& rhs) const {                    // line < line
            long double xa = this->x_intercept(sweep_line_y);
            long double xb = rhs.x_intercept(sweep_line_y);
            if(!CLOSE(xa,xb)){
                return xa<xb;
            }else{
                return angle_cmp(rhs);
            }
        }
        bool operator< (const Point& rhs) const {                   // line < event
            long double xa = this->x_intercept(rhs.y);
            long double xb = rhs.x;
            if(!CLOSE(xa,xb)){
                return xa<xb;
            }else{
                return false;
                /*
                if event on line, we define 
                event < line to be true,
                convenient for calling 
                T.upper_bound(event)
                */
            }
        }
        friend bool operator< (const Point& lhs, const Line& rhs) { // event < line
            long double xa = lhs.x;
            long double xb = rhs.x_intercept(lhs.y);
            if(!CLOSE(xa,xb)){
                return xa<xb;
            }else{
                return true;
                /*
                if event on line, we define 
                event < line to be true,
                convenient for calling 
                T.upper_bound(event)
                */
            }
        }
        void insert_in(std::map<Point, std::vector<Line> >& Q) const {
            Q.insert({Point(lower), std::vector<Line>{}});
            auto status = Q.insert({Point(upper), std::vector<Line>{*this}});
            if(!status.second){     // if point already in Q, update its U (std::vector<Line>)
                status.first->second.push_back(*this);
            }
        }
        bool is_lower(long double x, long double y) const {
            return CLOSE(lower.x, x) && CLOSE(lower.y, y);
        }
        
        // returns whether there is intersection and if there is, puts it into E
        // l1.intersect(l2) is different from l2.intersect(l1) in case of horizontal lines
        bool intersect(const Line& rhs, Point& E) const {
            const auto& p1 = upper;
            const auto& p2 = lower;
            const auto& p3 = rhs.upper;
            const auto& p4 = rhs.lower;
            if(CLOSE(p3.y, p4.y)){
                return false;
            }
            if(CLOSE(p1.y, p2.y)){
                long double x = rhs.x_intercept(p2.y);
                if(x > p2.x){return false;}
                E = Point(x, p2.y); return true;
            }
            long double m1 = (p1.x - p2.x)/(p1.y - p2.y);
            long double m2 = (p3.x - p4.x)/(p3.y - p4.y);
            if(CLOSE(m1,m2)){return false;}
            long double Y = (m1*p1.y - p1.x + p3.x - m2*p3.y)/(m1-m2);
            long double X = p1.x + m1*(Y-p1.y);
            if(Y > p2.y || Y > p4.y){return false;} // sfml y-axis is inverted
            E = Point(X,Y); return true;
        }
        // void print() const { // debug
        //     std::cout << "\t(" << upper.x << ", " << upper.y << ") (" << lower.x << ", " << lower.y << ")" << std::endl;
        //     return;
        // }
};

class Intersection : public sf::Drawable{
        sf::CircleShape dot;
        std::vector<Line> lines;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            target.draw(dot, states);
        }
    public:
        Intersection(Point e, std::vector<Line> lines){
            this->lines = lines;
            long double r = 2.f;
            dot = sf::CircleShape(r, 10);
            dot.setPosition(e.x-r, e.y-r);
            dot.setFillColor(sf::Color::White);
            dot.setOutlineThickness(8.f);
            dot.setOutlineColor(sf::Color::Blue);
        }
};

std::vector<Line> lines;
std::vector<Intersection> intersections;

std::map<Point, std::vector<Line> > Q; // (key,value) is (point, U)
std::set<Line, std::less<> > T;

void checkIntersection(std::set<Line>::iterator l1, std::set<Line>::iterator l2, long double y){
    if(l1==T.end() || l2==T.end()){return;}
    Point E(0,0);
    bool found = l1->intersect(*l2, E);
    if(!found || E.y < y){return;}
    Q.insert({E, std::vector<Line>{}});
    return;
}

void handleEvent(std::pair<Point, std::vector<Line> > e){
    auto& event = e.first;
    auto& U = e.second;
    std::vector<Line> C;
    std::vector<Line> L;    // don't need this if don't want to report lines of every intersection
    
    auto sl = T.end();     // left neighbour 
    auto sr = T.end();     // right neighbour
    auto s_ = T.end();     // leftmost in U+C  (s')
    auto s__ = T.end();    // rightmost in U+C (s")

    auto erase_begin = T.end(); // start of L+C in T
    // auto erase_end = T.end();   // same as sr

    auto it = T.upper_bound(event); // first element of L+C according to ordering before sweep line update
    
    if(it!=T.begin()){sl = std::prev(it);}

    for( ; it != T.end(); ++it){
        if(!CLOSE(it->x_intercept(event.y), event.x)){break;}
        
        if(erase_begin == T.end()){erase_begin = it;}

        bool in_L = it->is_lower(event.x, event.y);
        if(in_L){L.push_back(*it);}
        else    {C.push_back(*it);}
    }
    sr = it;    // may be reassigned to T.end()

    if(U.size() + L.size() + C.size() >= 2){
        L.insert(L.end(), U.begin(), U.end()); // intersection lines
        L.insert(L.end(), C.begin(), C.end());
        intersections.push_back(Intersection(event, L));
    }

    // remove L, C
    if(erase_begin != T.end()){T.erase(erase_begin, sr);}
    // add U, C after updating sweep_line_y
    sweep_line_y =  event.y;
    for(auto line : U){T.insert(line);}
    for(auto line : C){T.insert(line);}

    // find s' and s" (if they exist)
    if(U.size()+C.size()){
        if(sl != T.end()){
            s_ = std::next(sl);
        }else{
            s_ = T.begin();
        }
        s__ =  std::prev(sr);
    }

    if((U.size() + C.size()) == 0){
        checkIntersection(sl, sr, event.y);
    }else{
        checkIntersection(sl, s_, event.y);
        checkIntersection(s__, sr, event.y);
    }

    return;

}

void find_intersections(){
    Q.clear();
    T.clear();
    for(auto line: lines){
        line.insert_in(Q);  // create events
    }
    while(!Q.empty()){
        auto e = *Q.begin();
        Q.erase(Q.begin());
        handleEvent(e);
    }
}