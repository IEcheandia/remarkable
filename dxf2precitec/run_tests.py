import subprocess
from pathlib import Path
import os
import asyncio

this_dir = Path(__file__).resolve().parent
tests_dir = this_dir / 'tests'
summary_html = tests_dir / 'summary.html'
dxf2precitec = Path.home() / 'wm_inst/opt/wm_inst/bin/dxf2precitec'

def find_freeCAD():
    assert(os.name == 'nt') # this is a windows-only function
    import winreg
    import re
    val = winreg.QueryValue(winreg.HKEY_CLASSES_ROOT, r"FreeCAD.Document\Shell\open\command")
    return Path(re.match(r'"(.*?)" "%1"', val).group(1))

async def cmd(*args, **kwargs):
    args = [str(arg) for arg in args]
    p = await asyncio.create_subprocess_exec(*args, **kwargs)
    x = await p.wait()
    if x:
        raise RuntimeError(f'Command {str(args)} exited with {x}')

# NOTE: It turns out FreeCAD sucks! For example it does not support partial ellipses.
async def dxf2svg_via_freeCAD(dxf, svg):
    if os.name == 'nt':
        # for Windows locate FreeCAD and use the Python interpreter that comes with it
        free_cad = find_freeCAD()
        python = free_cad.parent / 'python.exe'
        await cmd(python, this_dir / 'dxf2svg_via_freecad.py', dxf, svg)
    else:
        # assuming FreeCAD is installed and available as module for the running Python interpreter
        import dxf2svg_via_freecad
        dxf2svg_via_freecad.dxf2svg(dxf, svg)

# To install dependencies: pip install ezdxf[draw]
def dxf2svg_via_ezdxf(dxf, svg):
    import matplotlib.pyplot as plt
    import ezdxf
    from ezdxf.addons.drawing import RenderContext, Frontend
    from ezdxf.addons.drawing.matplotlib import MatplotlibBackend

    doc = ezdxf.readfile(dxf)
    fig = plt.figure()
    fig.set_size_inches(4, 4)
    out = MatplotlibBackend(fig.add_axes([0, 0, 1, 1]), adjust_figure=False)
    Frontend(RenderContext(doc), out).draw_layout(doc.modelspace(), finalize=True)
    fig.savefig(svg)

async def task(dxf : Path,
               #free_cad_svg,
               ezdxf_svg,
               out_svg, out_json):
    print('Processing', dxf)

    #await dxf2svg_via_freeCAD(dxf, free_cad_svg)
    dxf2svg_via_ezdxf(dxf, ezdxf_svg)
    await cmd(dxf2precitec,
    '-svg', out_svg,
    '-e', '.1',
    '-optstart', '-optdir',
    '--',
    dxf, out_json)
    print('Done with', dxf)

async def main():
    tasks = list()

    with open(summary_html, 'w') as f:
        f.write('<html><table border="1"><tr><th>ezdxf</th><th>dxf2precitec</th></tr>\n')

        for dxf in sorted(tests_dir.glob('*.dxf')):

            p = dxf.with_suffix('')
            #free_cad_svg = Path(f'{p}_FreeCAD.svg')
            ezdxf_svg = Path(f'{p}_ezdxf.svg')
            out_svg = Path(f'{p}_out.svg')
            out_json = dxf.with_suffix('.json')

            fns = (
                #free_cad_svg,
                ezdxf_svg,
                out_svg)

            f.write(f'<tr><td colspan="{len(fns)}">{dxf.name}</td></tr>\n')
            f.write('<tr>\n')
            for svg in fns:
                f.write(f'  <td><img src="{svg.name}" /></td>\n')
            f.write('</tr>\n')

            tasks.append(task(dxf,
                              #free_cad_svg,
                              ezdxf_svg,
                              out_svg, out_json))

        f.write('</table></html>\n')

    await asyncio.gather(*tasks)


if __name__ == '__main__':
    asyncio.run(main())
    print('Visually check output to verify it looks good:', summary_html)
