vim.lsp.config.clangd = {
  cmd = { "clangd" },
  filetypes = { 'c', 'c.doxygen', 'cpp', 'cpp.doxygen', 'objc', 'objcpp', 'cuda' },
  root_markers = { ".editorconfig", "compile_commands.json", "compile_flags.txt", ".git", ".jj" },
}

vim.lsp.enable("clangd")
