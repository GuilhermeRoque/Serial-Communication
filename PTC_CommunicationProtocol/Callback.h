// classe abstrata para os callbacks do poller
class Callback {
public:
    // fd: descritor de arquivo a ser monitorado. Se < 0, este callback é um timer
    // tout: timeout em milissegundos. Se < 0, este callback não tem timeout
    Callback(int fd, long tout);

    // cria um callback para um timer (equivalente ao construtor anterior com fd=-1)
    // out: timeout
    Callback(long tout);

    virtual ~Callback();

    // ao especializar esta classe, devem-se implementar estes dois métodos !
    // handle: trata o evento representado neste callback
    // handle_timeout: trata o timeout associado a este callback
    virtual void handle() = 0;
    virtual void handle_timeout() = 0;

    // operator==: compara dois objetos callback
    // necessário para poder diferenciar callbacks ...
    virtual bool operator==(const Callback & o) const;

    // getter para o descritor de arquivo a ser monitorado
    int filedesc() const;

    // getter do valor de timeout remanescente
    int timeout() const;

    // ajusta timeout restante
    void update(long dt);

    // recarrega valor de timeout inicial
    void reload_timeout();

    // desativa timeout
    void disable_timeout();

    // reativa timeout
    void enable_timeout();

    bool timeout_enabled() { return base_tout > 0; }

protected:
    int fd; // se < 0, este callback se torna um simples timer
    long tout;
    long base_tout;// milissegundos. Se <= 0, este callback não tem timeout
};
