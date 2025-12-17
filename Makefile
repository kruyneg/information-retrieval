PYTHON := python3
VENV := .venv
ACTIVATE := . $(VENV)/bin/activate
DOCKER_COMPOSE := docker compose
MONGO_CONTAINER := informationretrieval-mongodb-1
MONGO_VOLUME := mongo_data
MONGO_BACKUP_DIR := ./mongo_backup
REQUIREMENTS := ./parser/requirements.txt
MONGO_URL := mongodb://user:password@localhost:27017/


$(VENV)/bin/activate:
	$(PYTHON) -m venv $(VENV)
	@echo "✅ Виртуальное окружение создано."

venv: $(VENV)/bin/activate

check-deps: venv
	$(ACTIVATE) && pip install -r $(REQUIREMENTS)
	@echo "✅ Все зависимости установлены."

mongo-up:
	$(DOCKER_COMPOSE) up -d mongodb
	@echo "🚀 MongoDB запущена."

mongo-down:
	$(DOCKER_COMPOSE) down
	@echo "🛑 MongoDB остановлена."

mongo-dump:
	mkdir -p $(MONGO_BACKUP_DIR)
	docker exec -i $(MONGO_CONTAINER) mongodump \
		--username $(shell echo $(MONGO_URL) | sed -E 's|mongodb://([^:]+):.*|\1|') \
		--password $(shell echo $(MONGO_URL) | sed -E 's|mongodb://[^:]+:([^@]+)@.*|\1|') \
		--authenticationDatabase admin \
		--db parser_db \
		--collection articles \
		--out /dump
	docker cp $(MONGO_CONTAINER):/dump $(MONGO_BACKUP_DIR)
	@echo "💾 Резервная копия MongoDB сохранена в $(MONGO_BACKUP_DIR)"

mongo-restore:
	docker cp $(MONGO_BACKUP_DIR)/dump $(MONGO_CONTAINER):/restore
	docker exec -i $(MONGO_CONTAINER) mongorestore \
		--username $(shell echo $(MONGO_URL) | sed -E 's|mongodb://([^:]+):.*|\1|') \
		--password $(shell echo $(MONGO_URL) | sed -E 's|mongodb://[^:]+:([^@]+)@.*|\1|') \
		--authenticationDatabase admin \
		--db parser_db \
		--collection articles \
		--drop \
		/restore/parser_db/articles.bson
	@echo "♻️  Восстановление MongoDB завершено из $(MONGO_BACKUP_DIR)"

run-parser:
	$(ACTIVATE) && $(PYTHON) ./parser/load_pages.py ./parser/configs/config.yml

start: mongo-up run-parser

stop: mongo-down

clean:
	rm -rf $(VENV)
	$(DOCKER_COMPOSE) down -v
	rm -rf $(MONGO_BACKUP_DIR)
	@echo "🧹 Всё очищено."

build-dicts:
	$(ACTIVATE) && cd engine && $(PYTHON) scripts/build_dicts.py

help:
	@echo ""
	@echo "Доступные команды:"
	@echo "  make venv         — создать виртуальное окружение"
	@echo "  make check-deps   — проверить и установить зависимости"
	@echo "  make mongo-up     — запустить MongoDB в Docker"
	@echo "  make mongo-down   — остановить MongoDB"
	@echo "  make mongo-dump   — создать бэкап mongo_data"
	@echo "  make run-parser   — запустить Python Parser"
	@echo "  make start        — запустить всё (Mongo + Python)"
	@echo "  make stop         — остановить всё"
	@echo "  make clean        — очистить окружение, volume и бэкапы"
	@echo ""

