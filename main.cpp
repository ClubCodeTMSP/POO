#include <iostream>
#include <memory>
#include <utility>

// Type pour notre DSL
enum class Etat{NO_STATE, EMPOISONNER};

struct Coordonnee {
    int x;
    int y;
};

class Guerrier;
class Magicien;
class MagicienGuerrier;

// Design pattern visitor
class Consommable {
public:
    // Maybe it is better to use type erasure
#define CONSOMMER_PAR(type) virtual bool consommerPar(type &) {return false;}
    CONSOMMER_PAR(Guerrier)
    CONSOMMER_PAR(Magicien)
    CONSOMMER_PAR(MagicienGuerrier)
#undef CONSOMMER_PAR
    virtual ~Consommable() = default;
};

// Design pattern visitor avec Consommable
class Nourrissable {
public:
    virtual void nourrir(Consommable &) = 0;

    virtual ~Nourrissable() = default;
private:
};

// Sorte d'interface pour les personnes attaquable
class Attaquable {
public:
    Attaquable(int vie) : mVie(vie){}

    int getVie() {
        return mVie;
    }

    Etat getEtat() {
        return mEtat;
    }

    virtual void prendreDommage(int value) {
        mVie -= value;
        if(mVie < 0)
            mVie = 0;
    }

    void increaseVie(int value) {
        mVie += value;
    }

    virtual void changerEtat(Etat etat) {
        mEtat = etat;
    }

    virtual ~Attaquable() = default;

protected:
    Etat mEtat = Etat::NO_STATE;
    int mVie;
};

// Interface pour attaquant
class Attaquant {
public:
    virtual void attaquer(Attaquable &cible) = 0;

    virtual ~Attaquant() = default;
};

class Deplacable {
public:
    Deplacable(Coordonnee position) : mPosition(position) {}
    virtual void moveTo(Coordonnee position) {
        mPosition = position;
    }

    virtual ~Deplacable() = default;

protected:
    Coordonnee mPosition;
};

class Detailable {
public:
    virtual void ecrireDetails(std::ostream &stream) = 0;

    virtual ~Detailable() = default;
};

class Utilisable {
public:
    virtual void utiliserSur(Attaquable &attaquable) = 0;
    virtual ~Utilisable() = default;
};

class Sort : public Utilisable, public Detailable {
public:
    void utiliserSur(Attaquable &attaquable) {
        attaquable.prendreDommage(10);
    }

    void ecrireDetails(std::ostream &stream) {
        stream << "Sort" << std::endl;
    }
};

class Arme : public Utilisable, public Detailable {
public:
};

class EpeeEmpoisonee : public Arme {
    virtual void utiliserSur(Attaquable &attaquable) {
        attaquable.prendreDommage(100);
        attaquable.changerEtat(Etat::EMPOISONNER);
    }

    void ecrireDetails(std::ostream &stream) {
        stream << "Epee empoisonnee" << std::endl;
    }
};


class GuerrierAbstrait {
public:
    GuerrierAbstrait() = default;

    void changeArme(std::unique_ptr<Arme> &&arme) {
        mArme = std::move(arme);
    }

    virtual ~GuerrierAbstrait() = default;

protected:
    std::unique_ptr<Arme> mArme;
};

class MagicienAbstrait {
public:
    void increaseMana(int value) {
        mMana += value;
    }

    virtual ~MagicienAbstrait() = default;

protected:
    int mMana = 50;
    Sort mSort;
};

class PersonnageJoueur : public Nourrissable, public Attaquant, public Attaquable, public Deplacable, public Detailable {
public:
    PersonnageJoueur(int vie, int x, int y) :
        Attaquable(vie),
        Deplacable(Coordonnee{x, y}) {

    }

    void ecrireDetails(std::ostream &stream) {
        stream << "Position : " << mPosition.x << ", " << mPosition.y << std::endl;
        if(mEtat == Etat::EMPOISONNER)
            stream << "Empoisonnee" << std::endl;
        stream << "Vie : " << mVie << std::endl;
    }
};

class Guerrier : public PersonnageJoueur, public GuerrierAbstrait {
public:
    Guerrier(int x, int y) : PersonnageJoueur(500, x, y) {}
    void nourrir(Consommable &consommable) {
        consommable.consommerPar(*this);
    }

    void attaquer(Attaquable &attaquable) {
        mArme->utiliserSur(attaquable);
    }

    void ecrireDetails(std::ostream &stream) {
        stream << "Guerrier :" << std::endl;
        stream << "Arme : ";
        mArme->ecrireDetails(stream);
        PersonnageJoueur::ecrireDetails(stream);
    }
};

class Magicien : public PersonnageJoueur, public MagicienAbstrait {
public:
    Magicien(int x, int y) : PersonnageJoueur(150, x, y) {}

    void nourrir(Consommable &consommable) {
        consommable.consommerPar(*this);
    }

    void attaquer(Attaquable &attaquable) {
        mMana -= 10;
        mSort.utiliserSur(attaquable);
    }

    void ecrireDetails(std::ostream &stream) {
        stream << "Magicien :" << std::endl;
        stream << "Sort : ";
        mSort.ecrireDetails(stream);
        PersonnageJoueur::ecrireDetails(stream);
    }
};

class MagicienGuerrier : public PersonnageJoueur, public MagicienAbstrait, public GuerrierAbstrait {
    void nourrir(Consommable &consommable) {
        consommable.consommerPar(*this);
    }
};

class PotionDeVie : public Consommable {
public:
#define CONSOMMER_PAR(type) bool consommerPar(type &attaquable) {\
    attaquable.increaseVie(10); \
    return true;\
}
    CONSOMMER_PAR(Magicien)
    CONSOMMER_PAR(Guerrier)
    CONSOMMER_PAR(MagicienGuerrier)
#undef CONSOMMER_PAR
};

class PotionDeMana : public Consommable {
public:
#define CONSOMMER_PAR(type) bool consommerPar(type &magicien) {\
        magicien.increaseMana(10);\
        return true;\
    }

    CONSOMMER_PAR(Magicien)
    CONSOMMER_PAR(MagicienGuerrier)
};

int main() {
    Guerrier guerrier(10, 10);
    Magicien magicien(5, 5);

    guerrier.changeArme(std::make_unique<EpeeEmpoisonee>());

    guerrier.ecrireDetails(std::cout);
    std::cout << std::endl;
    magicien.ecrireDetails(std::cout);
    std::cout << std::endl;
    guerrier.attaquer(magicien);
    magicien.attaquer(guerrier);
    std::cout << std::endl;
    guerrier.ecrireDetails(std::cout);
    std::cout << std::endl;
    magicien.ecrireDetails(std::cout);

    return 0;
}
