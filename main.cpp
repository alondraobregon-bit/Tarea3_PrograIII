#include <iostream>
#include <list>
#include <functional>
#include <map>
#include <string>
using namespace std;

using Command = std::function<void(const std::list<std::string>&)>;

class Entity {
    int vida;
    int x;
    int y;
public:
    Entity() {
        vida = 100;
        x = 0;
        y = 0;
    }
    void move(int _x, int _y) {
        x += _x;
        y += _y;
    }
    void heal(int n) {
        vida += n;
    }
    void damage(int n) {
        vida -= n;
    }
    void reset() {
        vida = 100;
        x = 0;
        y = 0;
    }
    void status() {
        cout << "vida: " << vida << endl;
        cout << "Posicion en el eje x: " << x << endl;
        cout << "Posicion en el eje y: " << y << endl;
    }
    string getState() const {
        return "vida=" + to_string(vida) +
               ", x=" + to_string(x) +
               ", y=" + to_string(y);
    }
};

void healFunctLibre(Entity& e, const list<string>& args) {
    int n = stoi(args.front());
    e.heal(n);
}

class DamageFunctor {
    Entity& e;
    int contador;
public:
    DamageFunctor(Entity& e_) : e(e_), contador(0) {}
    void operator()(const list<string>& args) {
        if (args.size() != 1) {
            cout << "Error: damage necesita 1 argumento" << endl;
            return;
        }
        int n;
        try {
            n = stoi(args.front());
        } catch (...) {
            cout << "Error: argumento debe ser numerico" << endl;
            return;
        }
        e.damage(n);
        contador++;
    }
};

class CommandCenter {
    map<string, Command> commands;
    list<string> history;
    Entity& entity;
    map<string,list<pair<string,list<string>>>> macros;
    public:
    CommandCenter(Entity& e): entity(e) {}
    void registerCommand(const string& nombre, Command command) {
        commands[nombre] = command;
    }
    void execute(const string& nombre, const list<string>& args) {
        map<string, Command>::iterator it = commands.find(nombre);
        if (it == commands.end()) {
            cout << nombre << ": Ese comando no existe" << endl;
            return;
        }
        string antes = entity.getState();
        it->second(args);
        string despues = entity.getState();
        string h = nombre + " antes: " + antes + " despues: " + despues;
        history.push_back(h);
    }
    void removeCommand(const string& nombre) {
        map<string, Command>::iterator it = commands.find(nombre);
        if (it == commands.end()) {
            cout << nombre << ": Ese comando no existe" << endl;
            return;
        }
        commands.erase(it);
        cout << "Comando eliminado: " << nombre << endl;
    }
    void registerMacro(const string& nombre, const list<pair<string,list<string>>>& pasos) {
        macros[nombre] = pasos;
        cout << "Se registro el macro: " << nombre << endl;
    }
    void executeMacro(const string& name) {
        map<string, list<pair<string, list<string>>>>::iterator it;
        it = macros.find(name);
        if (it == macros.end()) {
            cout << "Macro '" << name << "' no existe" << endl;
            return;
        }
        list<pair<string, list<string>>>::iterator paso;
        for (paso = it->second.begin(); paso != it->second.end(); ++paso) {
            string comando = paso->first;
            list<string> args = paso->second;

            map<string, Command>::iterator buscador = commands.find(comando);

            if (buscador == commands.end()) {
                cout << "Error en macro '" << name
                     << "': comando '" << comando
                     << "' no existe" << endl;
                return;
            }
            execute(comando, args);
        }
    }
};

int main() {
    Entity entity;
    CommandCenter center(entity);
    DamageFunctor dmg(entity);
    center.registerCommand("move",
        [&entity](const list<string>& args) {
            if (args.size() != 2) {
                cout << "Error: move necesita 2 argumentos" << endl;
                return;
            }
            auto it = args.begin();
            int x, y;
            try {
                x = stoi(*it++);
                y = stoi(*it);
            } catch (...) {
                cout << "Error: argumentos deben ser numeros" << endl;
                return;
            }
            entity.move(x,y);
        });
    center.registerCommand("reset",
        [&entity](const list<string>& args) {
            if (!args.empty()) {
                cout << "Error: reset no debe recibir ningun argumento" << endl;
                return;
            }
            entity.reset();
        });
    center.registerCommand("status",
        [&entity](const list<string>& args) {
            if (!args.empty()) {
                cout << "Error: status no debe recibir ningun argumento" << endl;
                return;
            }
            entity.status();
        });
    center.registerCommand("heal",
        [&entity](const list<string>& args) {
            if (args.size() != 1) {
            cout << "Error: heal necesita 1 argumento" << endl;
            return;
        }
            try {
            stoi(args.front());
        } catch (...) {
            cout << "Error: argumentos deben ser numericos" << endl;
            return;
        }
            healFunctLibre(entity, args);
        });
    center.registerCommand("damage",dmg);

    cout << "------------------" << endl;
    cout << "Comandos Validos" << endl;
    cout << "------------------" << endl;

    center.execute("move", {"5", "3"});
    center.execute("heal", {"20"});
    center.execute("damage", {"10"});
    center.execute("status", {});
    cout << endl;

    cout << "------------------" << endl;
    cout << "Comandos Invalidos" << endl;
    cout << "------------------" << endl;

    center.execute("move", {"5"});
    center.execute("heal", {"abc"});
    cout << "Ejecutar comando 'volar'" << endl;
    center.execute("volar", {});

    cout << endl;
    cout << "------------------" << endl;
    cout << "Registro de Macros" << endl;
    cout << "------------------" << endl;

    list<pair<string, list<string>>> m1 = {
        {"heal", {"10"}},
        {"status", {}}
    };

    center.registerMacro("curarse", m1);

    list<pair<string, list<string>>> m2 = {
        {"move", {"2", "2"}},
        {"damage", {"15"}},
        {"status", {}}
    };

    center.registerMacro("batalla", m2);
    list<pair<string, list<string>>> m3 = {
        {"reset", {}},
        {"status", {}}
    };

    center.registerMacro("reiniciar", m3);
    cout << endl;
    cout << "--------------------" << endl;
    cout << "Ejecucion de Macros" << endl;
    cout << "--------------------" << endl;

    center.executeMacro("curarse");
    center.executeMacro("batalla");
    center.executeMacro("reiniciar");
    //Aquí mostrará un error, ya que volar, no es un macro registrado
    center.executeMacro("volar");

    cout << endl;
    cout << "Eliminando comando 'heal'" << endl;
    center.removeCommand("heal");

    //Mostrará un error porque heal no existe dentro del map
    center.execute("heal", {"10"});
    cout << endl;
    //Aquí derivará a un error, ya que el comando heal no existe y el macro "curarse" necesita a heal
    cout << "------------------" << endl;
    cout << "Macro Fallido" << endl;
    cout << "------------------" << endl;
    center.executeMacro("curarse");

    cout << endl;
    cout << "------------------" << endl;
    cout << "Estado Final" << endl;
    cout << "------------------" << endl;

    center.execute("status", {});
    return 0;
}