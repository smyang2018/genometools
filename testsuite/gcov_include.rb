full_coverage_files = [ "src/libgtext/chseqids_stream.c",
                        "src/tools/gt_chseqids.c"
                      ]

full_coverage_files.each do |file|
  base = File.basename(file)
  Name "full coverage for #{file}"
  Keywords "gcov"
  Test do
    run "cd #{$cur} && gcov -o obj/#{file} #{file} && cd -"
    run "mv #{$cur}/#{base}.gcov ."
    grep(base+".gcov", "#####", true)
  end
end
