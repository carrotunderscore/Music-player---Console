#include <SFML/Graphics.hpp>



class Graph {
private:
    
public:
    //Constructor
    Graph(){

    }


    void plotSineWave(std::vector<double> data) {

        sf::RenderWindow window(sf::VideoMode(2500, 600), "Sine Wave Plot");
        window.setFramerateLimit(60);

        // Scaling
        float scaleX = static_cast<float>(window.getSize().x) / (data.size() - 1);
        float scaleY = window.getSize().y / 100000.0f;  // Decreasing the scale for a more "zoomed out" look

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            window.clear(sf::Color::White);

            for (size_t i = 0; i < data.size() - 1; i++) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(i * scaleX, window.getSize().y / 2 - data[i] * scaleY), sf::Color::Blue),
                    sf::Vertex(sf::Vector2f((i + 1) * scaleX, window.getSize().y / 2 - data[i + 1] * scaleY), sf::Color::Blue)
                };

                window.draw(line, 2, sf::Lines);
            }

            window.display();
        }
    }

    


};