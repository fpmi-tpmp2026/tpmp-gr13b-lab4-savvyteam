DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS arrangement_compositions;
DROP TABLE IF EXISTS arrangements;
DROP TABLE IF EXISTS flowers;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS config;

CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    last_name VARCHAR(50) NOT NULL,
    first_name VARCHAR(50) NOT NULL,
    middle_name VARCHAR(50),
    phone VARCHAR(20) UNIQUE,
    email VARCHAR(100) UNIQUE,
    registration_date DATE NOT NULL,
    is_active BOOLEAN DEFAULT 1
);

CREATE TABLE flowers (
    flower_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    variety VARCHAR(100) NOT NULL,
    price_per_unit DECIMAL(10,2) NOT NULL
);

CREATE TABLE arrangements (
    arrangement_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(150) NOT NULL
);

CREATE TABLE arrangement_compositions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    arrangement_id INTEGER NOT NULL,
    flower_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL CHECK (quantity > 0),
    FOREIGN KEY (arrangement_id) REFERENCES arrangements(arrangement_id) ON DELETE CASCADE,
    FOREIGN KEY (flower_id) REFERENCES flowers(flower_id) ON DELETE CASCADE
);

CREATE TABLE orders (
    order_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    arrangement_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL CHECK (quantity > 0),
    order_date DATE NOT NULL,
    completion_date DATE,
    total_price DECIMAL(12,2),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (arrangement_id) REFERENCES arrangements(arrangement_id) ON DELETE CASCADE
);

CREATE TABLE config (
    config_id INTEGER PRIMARY KEY AUTOINCREMENT,
    config_key VARCHAR(100) UNIQUE NOT NULL,
    config_value VARCHAR(255) NOT NULL,
    description TEXT
);

INSERT INTO users (last_name, first_name, middle_name, phone, email, registration_date, is_active) VALUES
('Иванова', 'Анна', 'Петровна', '+375291234567', 'anna@email.com', '2024-01-15', 1),
('Петров', 'Сергей', 'Иванович', '+375292345678', 'sergey@email.com', '2024-02-20', 1),
('Сидорова', 'Елена', 'Владимировна', '+375293456789', 'elena@email.com', '2024-03-10', 1),
('Козлов', 'Дмитрий', 'Александрович', '+375294567890', 'dmitry@email.com', '2024-01-05', 1),
('Морозова', 'Татьяна', 'Николаевна', '+375295678901', 'tatiana@email.com', '2024-03-25', 0),
('Admin', 'Admin', '', '+375290000000', 'admin@flower.com', '2024-01-01', 1);

INSERT INTO flowers (name, variety, price_per_unit) VALUES
('Роза', 'Эсперанца', 5.50),
('Роза', 'Фридом', 6.00),
('Тюльпан', 'Розовый', 3.20),
('Тюльпан', 'Желтый', 3.00),
('Хризантема', 'Белая', 4.50),
('Хризантема', 'Бордовая', 4.80),
('Лилия', 'Белая', 7.00),
('Лилия', 'Розовая', 7.50),
('Гербера', 'Оранжевая', 4.00),
('Гербера', 'Красная', 4.20);

INSERT INTO arrangements (name) VALUES
('Весеннее настроение'),
('Классический букет'),
('Романтический'),
('Для любимой'),
('Солнечный день'),
('Элегантность');

INSERT INTO arrangement_compositions (arrangement_id, flower_id, quantity) VALUES
(1, 3, 5),
(1, 4, 3),
(1, 9, 2),
(2, 1, 7),
(2, 2, 3),
(2, 7, 2),
(3, 1, 5),
(3, 2, 5),
(3, 8, 2),
(4, 2, 11),
(4, 8, 3),
(5, 4, 6),
(5, 9, 4),
(5, 10, 3),
(6, 7, 5),
(6, 8, 5),
(6, 5, 4);

INSERT INTO orders (user_id, arrangement_id, quantity, order_date, completion_date, total_price) VALUES
(1, 1, 2, '2024-03-01', '2024-03-02', NULL),
(2, 2, 1, '2024-03-05', '2024-03-07', NULL),
(3, 3, 3, '2024-03-10', '2024-03-13', NULL),
(1, 4, 1, '2024-03-15', '2024-03-16', NULL),
(4, 5, 2, '2024-03-20', '2024-03-22', NULL),
(5, 6, 1, '2024-03-25', NULL, NULL);

INSERT INTO config (config_key, config_value, description) VALUES
('SURCHARGE_1_DAY_PERCENT', '25', 'Надбавка при выполнении заказа в течение 1 суток (%)'),
('SURCHARGE_2_DAY_PERCENT', '15', 'Надбавка при выполнении заказа в течение 2 суток (%)'),
('MAX_PREPARE_DAYS', '5', 'Максимальное количество дней для выполнения заказа'),
('DEFAULT_CURRENCY', 'BYN', 'Валюта расчетов');

UPDATE orders SET total_price = (
    SELECT SUM(f.price_per_unit * ac.quantity) * o.quantity
    FROM arrangement_compositions ac
    JOIN flowers f ON ac.flower_id = f.flower_id
    WHERE ac.arrangement_id = o.arrangement_id
) WHERE completion_date IS NULL;

UPDATE orders SET total_price = (
    SELECT SUM(f.price_per_unit * ac.quantity) * o.quantity *
    CASE 
        WHEN julianday(o.completion_date) - julianday(o.order_date) <= 1 
            THEN 1 + (SELECT CAST(config_value AS REAL) / 100 FROM config WHERE config_key = 'SURCHARGE_1_DAY_PERCENT')
        WHEN julianday(o.completion_date) - julianday(o.order_date) <= 2 
            THEN 1 + (SELECT CAST(config_value AS REAL) / 100 FROM config WHERE config_key = 'SURCHARGE_2_DAY_PERCENT')
        ELSE 1
    END
    FROM arrangement_compositions ac
    JOIN flowers f ON ac.flower_id = f.flower_id
    WHERE ac.arrangement_id = o.arrangement_id
) WHERE completion_date IS NOT NULL;