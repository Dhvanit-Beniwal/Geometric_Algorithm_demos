#define INF std::numeric_limits<float>::max()
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

// is a->b->c a right turn?
bool right_turn(const sf::Vertex a, const sf::Vertex b, const sf::Vertex c){
    /*
        cross>0 if left turn a->b->c
        cross<0 if right turn a->b->c
        cross=0 if collinear 
    */
    int cross = (b.position.x-a.position.x)*(c.position.y-b.position.y) - (b.position.y-a.position.y)*(c.position.x-b.position.x);
    cross *= -1; // because sfml y-axis (computer graphics in general) is upside down
    if(!CLOSE(cross,0)){
        return cross<0;
    }else{
        int dot = (b.position.x-a.position.x)*(c.position.x-b.position.x) + (b.position.y-a.position.y)*(c.position.y-b.position.y);
        return dot<0 && !CLOSE(dot,0);
        // for collinear a-b-c right turn iff a->b->c is 180 deg (left turn if 0 deg)
        // a->a->c is a left turn (helps in function extremal_point when q is a point in chain)
    }
}

// returns sorted anti-clockwise
std::vector< sf::Vertex > grahamScan(std::vector< Point >::iterator begin, std::vector< Point >::iterator end){ 
    int n = end-begin;
    if(n<3){
        std::vector< sf::Vertex > final_hull;
        for(auto it = begin; it!= end; ++it){
            final_hull.push_back(it->vertex);
        }
        return final_hull;
    }
    
    std::vector< sf::Vertex > fhull;    // 'forward' hull
    auto it = begin;
    fhull.push_back(it->vertex); ++it;
    fhull.push_back(it->vertex); ++it;
    for(; it != end; ++it){
        fhull.push_back(it->vertex);
        while(fhull.size()>2 && !right_turn(fhull.end()[-3], fhull.end()[-2], fhull.end()[-1])){
            fhull.erase(fhull.end()-2);
        }
    }

    auto rbegin = std::make_reverse_iterator(end);
    auto rend = std::make_reverse_iterator(begin);

    std::vector< sf::Vertex > rhull; // other half ('reverse' hull)
    auto rit = rbegin;
    rhull.push_back(rit->vertex); ++rit;
    rhull.push_back(rit->vertex); ++rit;
    for(; rit != rend; ++rit){
        rhull.push_back(rit->vertex);
        while(rhull.size()>2 && !right_turn(rhull.end()[-3], rhull.end()[-2], rhull.end()[-1])){
            rhull.erase(rhull.end()-2);
        }
    }

    fhull.insert(fhull.end(), rhull.begin()+1, rhull.end()-1);
    return fhull;
}

// returns p such that entire chain is to the right of q->p
// if q is a part of chain returns prev(q) or earliest such point in the chain if q, prev_q, prev_prev_q are collinear etc
// binary search, O(log(chain.size))
sf::Vertex& extremal_point(std::vector< sf::Vertex > chain, sf::Vertex q){
    auto first = chain.begin();
    auto last = chain.end();
    
    while((first+1)!=last){
        if(first+2 == last){    // length 2 chain
            return right_turn(q, *first, *(first+1)) ? *first : *(first+1);
        }
        // now (last-first) is atleast 3

        bool is_prev_right = right_turn(q, *first, *(last-1));
        bool is_next_right = right_turn(q, *first, *(first+1));
        if(is_prev_right && is_next_right){
            return *first;
        }
        // which part is 'first' in
        bool in_farside = (is_prev_right && !is_next_right);
        bool in_nearside = (!is_prev_right && is_next_right);
        // bool at_opposite_extreme = (!is_prev_right && !is_next_right);

        auto middle = first + (last-first)/2;
        is_prev_right = right_turn(q, *middle, *(middle-1));
        is_next_right = right_turn(q, *middle, *(middle+1));
        if(is_prev_right && is_next_right){
            return *middle;
        }
        if(is_prev_right && !is_next_right){    // middle in farside, check if first was also in farside etc
            if(!in_farside || right_turn(q, *middle, *first)){
                first = middle+1;
            }else{
                last = middle;
            }
        }else
        if(!is_prev_right && is_next_right){    // middle in nearside, check if first was also in nearside etc
            if(!in_nearside || right_turn(q, *middle, *first)){
                last = middle;
            }else{
                first = middle+1;
            }
        }else{      // middle is at the other extremal point
            if(in_farside){
                last = middle;
            }else
            if(in_nearside){
                first = middle+1;
            }
        }
    }
    return *first;
}

// equality check
bool close(sf::Vertex a, sf::Vertex b){
    return CLOSE(a.position.x, b.position.x) && CLOSE(a.position.y, b.position.y);
}

// append q from Q to result such that [result[-2] -- result[-1] -- q] angle is maximum, linear scan
void add_max_angle_point(std::vector<sf::Vertex>& result, std::vector<sf::Vertex>& Q){
    float x1,y1,x2,y2;
    x2 = result.back().position.x;
    y2 = result.back().position.y;
    bool degenerate_case = (result.size()==1);
    if(degenerate_case){
        x1 = x2;
        y1 = INF;   // not actually used for computation/comparison, also should it be -INF because sfml y-axis is upside down?
    }else{
        x1 = result.end()[-2].position.x;
        y1 = result.end()[-2].position.y;
    }
    sf::Vertex* maxsofar = nullptr;
    float maxval = -INF;
    for(auto& q : Q){
        float tmp = degenerate_case ? -(y2-q.position.y) : (q.position.x-x2)*(x2-x1) + (q.position.y-y2)*(y2-y1);
        tmp /= sqrtf((q.position.x-x2)*(q.position.x-x2) + (q.position.y-y2)*(q.position.y-y2));
        if(tmp > maxval){
            maxval = tmp;
            maxsofar = &q;
        }
    }
    if(maxsofar){result.push_back(*maxsofar);}
}

// main algorithm
// expects sorted points
// returns empty vector on failure
std::vector< sf::Vertex > chan_algo(std::vector< Point > points, int m){
    int n = points.size();
    std::vector< std::vector<sf::Vertex> > hulls;
    for(int i=0, j=0; j<n ; ++i){
        j += m;
        if(j>n){j = n;}
        hulls.push_back(grahamScan(points.begin()+i*m, points.begin()+j));
    }
    
    auto& rightmost = points.back().vertex;
    std::vector<sf::Vertex> result{rightmost};

    for(int j = 0; j<m; ++j){
        std::vector<sf::Vertex> Q;
        for(auto hull : hulls){
            auto p = extremal_point(hull, result.back());
            
            // p must not be the same as the second last or last element in result
            if(!close(result.end()[-1], p) && ((result.size()==1) || (!close(result.end()[-2], p)))){
                Q.push_back(p);
            }
        }
        add_max_angle_point(result, Q); // result.push_back(max_angle_point(result[-1], result[-2], Q))
        if(close(result.back(), result.front())){return result;}
    }
    return std::vector< sf::Vertex >{};
}