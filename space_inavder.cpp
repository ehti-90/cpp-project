#include<SFML\Graphics.hpp>
#include<iostream> 

using namespace std;
using namespace sf;

int main(){

    RenderWindow window(VideoMode(400,400),"SFML EHTI90",Style::Close | Style::Resize);
    RectangleShape player(Vector2f(100.0f,100.0f));
    player.setFillColor(Color::Green);
    
    while (window.isOpen()){
        Event ev;
        while(window.pollEvent(ev)){
            
switch(ev.type){
case Event::Closed:
window.close();
break;

case Event::Resized:
cout<<"new window length : %i new window height %i\n"<<ev.size.width<<endl<<ev.size.height<<endl;
break;

case Event::TextEntered:
    if (ev.text.unicode < 128) {
        cout << static_cast<char>(ev.text.unicode) << endl; 
    }
   

}
}
if(Keyboard::isKeyPressed(Keyboard::S)){
    player.move(0.0f,0.1f);
}
if(Keyboard::isKeyPressed(Keyboard::W)){
    player.move(0.0f,-0.1f);
}if(Keyboard::isKeyPressed(Keyboard::D)){
    player.move(0.1f,0.0f);
}if(Keyboard::isKeyPressed(Keyboard::A)){
    player.move(-0.1f,0.0f);
}

window.clear();
window.draw(player);
window.display();




}

    return 0;

}