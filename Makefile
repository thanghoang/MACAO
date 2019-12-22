.PHONY: clean All

All:
	@echo "----------Building project:[ MACAO - Debug ]----------"
	@cd "MACAO" && "$(MAKE)" -f  "MACAO.mk"
clean:
	@echo "----------Cleaning project:[ MACAO - Debug ]----------"
	@cd "MACAO" && "$(MAKE)" -f  "MACAO.mk" clean
