# List PDF files here
OUTPUT= adder.pdf adder_toffoli.pdf ibm5q.pdf adder_interact.pdf adder_reverse.pdf swap.pdf reverse.pdf

%.pdf: %.eps
	epstopdf $<

%.pdf: %.fig
	fig2dev -L pdf $^ $@

%.eps: %.dia
	dia -t eps -e $@ $<

%-1.pdf: %.mp
	mptopdf $<
	rm -f $<

%.pdf: %-1.pdf
	mv $< $@

%.mp: %.mp.dia
	dia -t mp -e $@ $<

%.eps: %.plot
	gnuplot $<

#%.pdf: %.gv
#	dot -Tpdf -o $@ $<

%.png: %.dot
	dot -Tpng -o $@ $<

%.pdf: %.dot
	dot -Tpdf -o $@ $<

%.pdf: %.svg
	inkscape -A $@ $<

%.pdf: %.odg
	soffice --convert-to pdf $<	# Silently fails if soffice is already running. This is a feature. :(

%.pdf: %.qpic
	qpic -f pdf $<

%.png: %.pdf
#	convert -density 300 -resample 300 $< $@
	convert -density 150 -resample 150 $< $@

%_crop.pdf: %.pdf
	pdfcrop $< $@


all: $(OUTPUT)

clean:
	rm -rf $(OUTPUT) *.mpo *.mpx *.1 *.log *.mp

